/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#include "opentxs/stdafx.hpp"

#include "opentxs/consensus/ServerContext.hpp"

#include "opentxs/consensus/TransactionStatement.hpp"
#include "opentxs/core/Item.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/network/ServerConnection.hpp"

#define OT_METHOD "ServerContext::"

namespace opentxs
{
ServerContext::ServerContext(
    const ConstNym& local,
    const ConstNym& remote,
    const Identifier& server,
    ServerConnection& connection)
    : ot_super(local, remote, server)
    , connection_(connection)
    , highest_transaction_number_(0)
{
}

ServerContext::ServerContext(
    const proto::Context& serialized,
    const ConstNym& local,
    const ConstNym& remote,
    ServerConnection& connection)
    : ot_super(
          serialized,
          local,
          remote,
          Identifier(serialized.servercontext().serverid()))
    , connection_(connection)
    , highest_transaction_number_(
          serialized.servercontext().highesttransactionnumber())
{
    for (const auto& it : serialized.servercontext().tentativerequestnumber()) {
        tentative_transaction_numbers_.insert(it);
    }
}

bool ServerContext::AcceptIssuedNumber(const TransactionNumber& number)
{
    Lock lock(lock_);

    bool accepted = false;
    const bool tentative = remove_tentative_number(lock, number);

    if (tentative) {
        accepted = issue_number(lock, number);
    }

    return accepted;
}

bool ServerContext::AcceptIssuedNumbers(const TransactionStatement& statement)
{
    Lock lock(lock_);

    std::size_t added = 0;
    const auto offered = statement.Issued().size();

    if (0 == offered) {
        return false;
    }

    std::set<TransactionNumber> adding, accepted, rejected;

    for (const auto& number : statement.Issued()) {
        // If number wasn't already on issued list, then add to BOTH
        // lists. Otherwise do nothing (it's already on the issued list,
        // and no longer valid on the available list--thus shouldn't be
        // re-added thereanyway.)
        const bool tentative =
            (1 == tentative_transaction_numbers_.count(number));
        const bool issued = (1 == issued_transaction_numbers_.count(number));

        if (tentative && !issued) {
            adding.insert(number);
        }
    }

    // Looks like we found some numbers to accept (tentative numbers we had
    // already been waiting for, yet hadn't processed onto our issued list yet)
    if (!adding.empty()) {
        update_highest(lock, adding, accepted, rejected);

        // We only remove-tentative-num/add-transaction-num for the numbers
        // that were above our 'last highest number'. The contents of rejected
        // are thus ignored for these purposes.
        for (const auto& number : accepted) {
            tentative_transaction_numbers_.erase(number);

            if (issue_number(lock, number)) {
                added++;
            }
        }
    }

    return (added == offered);
}

bool ServerContext::AddTentativeNumber(const TransactionNumber& number)
{
    Lock lock(lock_);

    if (number < highest_transaction_number_.load()) {
        return false;
    }

    auto output = tentative_transaction_numbers_.insert(number);

    lock.unlock();

    return output.second;
}

bool ServerContext::finalize_server_command(Message& command) const
{
    OT_ASSERT(nym_);

    if (false == command.SignContract(*nym_)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to sign server message."
              << std::endl;

        return false;
    }

    if (false == command.SaveContract()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed to serialize server message." << std::endl;

        return false;
    }

    return true;
}

std::unique_ptr<TransactionStatement> ServerContext::generate_statement(
    const Lock& lock,
    const std::set<TransactionNumber>& adding,
    const std::set<TransactionNumber>& without) const
{
    OT_ASSERT(verify_write_lock(lock));

    std::set<TransactionNumber> issued;
    std::set<TransactionNumber> available;

    for (const auto& number : issued_transaction_numbers_) {
        const bool include = (0 == without.count(number));

        if (include) {
            issued.insert(number);
            available.insert(number);
        }
    }

    for (const auto& number : adding) {
        issued.insert(number);
        available.insert(number);
    }

    std::unique_ptr<TransactionStatement> output(
        new TransactionStatement(String(server_id_).Get(), issued, available));

    return output;
}

TransactionNumber ServerContext::Highest() const
{
    return highest_transaction_number_.load();
}

std::unique_ptr<Message> ServerContext::initialize_server_command(
    const MessageType type) const
{
    std::unique_ptr<Message> output(new Message);

    OT_ASSERT(output);
    OT_ASSERT(nym_);

    output->m_strCommand.Set(Message::Command(type).data());
    output->m_strNymID = String(nym_->ID());
    output->m_strNotaryID = String(server_id_);

    return output;
}

TransactionNumber ServerContext::NextTransactionNumber()
{
    Lock lock(lock_);

    if (0 == available_transaction_numbers_.size()) {
        return 0;
    }

    auto first = available_transaction_numbers_.begin();
    const auto output = *first;
    available_transaction_numbers_.erase(first);

    return output;
}

NetworkReplyMessage ServerContext::PingNotary()
{
    Lock lock(message_lock_);

    OT_ASSERT(nym_);

    auto request = initialize_server_command(MessageType::pingNotary);

    if (false == bool(request)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed to initialize server message." << std::endl;

        return {};
    }

    String serializedAuthKey{};
    String serializedEncryptKey{};
    const auto& authKey = nym_->GetPublicAuthKey();
    const auto& encrKey = nym_->GetPublicEncrKey();
    authKey.GetPublicKey(serializedAuthKey);
    encrKey.GetPublicKey(serializedEncryptKey);
    request->m_strRequestNum = std::to_string(FIRST_REQUEST_NUMBER).c_str();
    request->m_strNymPublicKey = serializedAuthKey;
    request->m_strNymID2 = serializedEncryptKey;
    request->keytypeAuthent_ = authKey.keyType();
    request->keytypeEncrypt_ = encrKey.keyType();

    if (false == finalize_server_command(*request)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed to finalize server message." << std::endl;

        return {};
    }

    return connection_.Send(*request);
}

bool ServerContext::remove_acknowledged_number(
    const Lock& lock,
    const Message& reply)
{
    OT_ASSERT(verify_write_lock(lock));

    std::set<RequestNumber> list{};

    if (false == reply.m_AcknowledgedReplies.Output(list)) {

        return false;
    }

    return remove_acknowledged_number(lock, list);
}

bool ServerContext::remove_tentative_number(
    const Lock& lock,
    const TransactionNumber& number)
{
    OT_ASSERT(verify_write_lock(lock));

    auto output = tentative_transaction_numbers_.erase(number);

    return (0 < output);
}

bool ServerContext::RemoveTentativeNumber(const TransactionNumber& number)
{
    Lock lock(lock_);

    return remove_tentative_number(lock, number);
}

void ServerContext::scan_number_set(
    const std::set<TransactionNumber>& input,
    TransactionNumber& highest,
    TransactionNumber& lowest)
{
    highest = 0;
    lowest = 0;

    if (0 < input.size()) {
        lowest = *input.cbegin();
        highest = *input.crbegin();
    }
}

proto::Context ServerContext::serialize(const Lock& lock) const
{
    OT_ASSERT(verify_write_lock(lock));

    auto output = serialize(lock, Type());
    auto& server = *output.mutable_servercontext();
    server.set_version(output.version());
    server.set_serverid(String(server_id_).Get());
    server.set_highesttransactionnumber(highest_transaction_number_.load());

    for (const auto& it : tentative_transaction_numbers_) {
        server.add_tentativerequestnumber(it);
    }

    return output;
}

bool ServerContext::SetHighest(const TransactionNumber& highest)
{
    Lock lock(lock_);

    if (highest >= highest_transaction_number_.load()) {
        highest_transaction_number_.store(highest);

        return true;
    }

    return false;
}

std::unique_ptr<Item> ServerContext::Statement(const OTTransaction& owner) const
{
    const std::set<TransactionNumber> empty;

    return Statement(owner, empty);
}

std::unique_ptr<Item> ServerContext::Statement(
    const OTTransaction& transaction,
    const std::set<TransactionNumber>& adding) const
{
    Lock lock(lock_);
    std::unique_ptr<Item> output;

    OT_ASSERT(nym_);

    if ((transaction.GetNymID() != nym_->ID())) {
        otErr << OT_METHOD << __FUNCTION__ << ": Transaction has wrong owner."
              << std::endl;

        return output;
    }

    // transaction is the depositPaymentPlan, activateSmartContract, or
    // marketOffer that triggered the need for this transaction statement.
    // Since it uses up a transaction number, I will be sure to remove that one
    // from my list before signing the list.
    output.reset(Item::CreateItemFromTransaction(
        transaction, Item::transactionStatement));

    // The above has an ASSERT, so this this will never actually happen.
    if (!output) {
        return output;
    }

    const std::set<TransactionNumber> empty;
    auto statement = generate_statement(lock, adding, empty);

    if (!statement) {
        return output;
    }

    switch (transaction.GetType()) {
        case OTTransaction::cancelCronItem: {
            if (transaction.GetTransactionNum() > 0) {
                statement->Remove(transaction.GetTransactionNum());
            }
        } break;
        // Transaction Statements usually only have a transaction number in the
        // case of market offers and payment plans, in which case the number
        // should NOT be removed, and remains in play until final closure from
        // Cron.
        case OTTransaction::marketOffer:
        case OTTransaction::paymentPlan:
        case OTTransaction::smartContract:
        default: {
        }
    }

    // What about cases where no number is being used? (Such as processNymbox)
    // Perhaps then if this function is even called, it's with a 0-number
    // transaction, in which case the above Removes probably won't hurt
    // anything.  Todo.

    output->SetAttachment(String(*statement));
    output->SignContract(*nym_);
    // OTTransactionType needs to weasel in a "date signed" variable.
    output->SaveContract();

    return output;
}

std::unique_ptr<TransactionStatement> ServerContext::Statement(
    const std::set<TransactionNumber>& adding,
    const std::set<TransactionNumber>& without) const
{
    Lock lock(lock_);

    return generate_statement(lock, adding, without);
}

proto::ConsensusType ServerContext::Type() const
{
    return proto::CONSENSUSTYPE_SERVER;
}

TransactionNumber ServerContext::update_highest(
    const Lock& lock,
    const std::set<TransactionNumber>& numbers,
    std::set<TransactionNumber>& good,
    std::set<TransactionNumber>& bad)
{
    OT_ASSERT(verify_write_lock(lock));

    TransactionNumber output = 0;  // 0 is success.
    TransactionNumber highest = 0;
    TransactionNumber lowest = 0;

    scan_number_set(numbers, highest, lowest);

    const TransactionNumber oldHighest = highest_transaction_number_.load();

    validate_number_set(numbers, oldHighest, good, bad);

    if ((lowest > 0) && (lowest <= oldHighest)) {
        // Return the first invalid number
        output = lowest;
    }

    if (!good.empty()) {
        if (0 != oldHighest) {
            otOut << OT_METHOD << __FUNCTION__
                  << ": Raising Highest Transaction Number "
                  << "from " << oldHighest << " to " << highest << "."
                  << std::endl;
        } else {
            otOut << OT_METHOD << __FUNCTION__
                  << ": Creating Highest Transaction Number "
                  << "entry for this server as '" << highest << "'."
                  << std::endl;
        }

        highest_transaction_number_.store(highest);
    }

    return output;
}

Identifier ServerContext::update_remote_hash(
    const Lock& lock,
    const Message& reply)
{
    OT_ASSERT(verify_write_lock(lock));

    Identifier output{};
    const auto& input = reply.m_strNymboxHash;

    if (input.Exists()) {
        output.SetString(input);
        remote_nymbox_hash_ = output;
    }

    return output;
}

TransactionNumber ServerContext::UpdateHighest(
    const std::set<TransactionNumber>& numbers,
    std::set<TransactionNumber>& good,
    std::set<TransactionNumber>& bad)
{
    Lock lock(lock_);

    return update_highest(lock, numbers, good, bad);
}

void ServerContext::validate_number_set(
    const std::set<TransactionNumber>& input,
    const TransactionNumber limit,
    std::set<TransactionNumber>& good,
    std::set<TransactionNumber>& bad)
{
    for (const auto& it : input) {
        if (it <= limit) {
            otWarn << OT_METHOD << __FUNCTION__ << ": New transaction number is"
                   << " less-than-or-equal-to last known 'highest trans number'"
                   << " record. (Must be seeing the same server reply for a "
                   << "second time, due to a receipt in my Nymbox.) FYI, last "
                   << "known 'highest' number received: " << limit
                   << " (Current 'violator': " << it << ") Skipping..."
                   << std::endl;
            bad.insert(it);
        } else {
            good.insert(it);
        }
    }
}

RequestNumber ServerContext::UpdateRequestNumber()
{
    bool notUsed{false};

    return UpdateRequestNumber(notUsed);
}

RequestNumber ServerContext::UpdateRequestNumber(bool& sendStatus)
{
    sendStatus = false;

    Lock lock(message_lock_);

    auto request = initialize_server_command(MessageType::getRequestNumber);

    if (false == bool(request)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed to initialize server message." << std::endl;

        return {};
    }

    request->m_strRequestNum = std::to_string(FIRST_REQUEST_NUMBER).c_str();

    if (false == finalize_server_command(*request)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed to finalize server message." << std::endl;

        return {};
    }

    const auto response = connection_.Send(*request);
    const auto& status = response.first;
    const auto& reply = response.second;

    switch (status) {
        case SendResult::TIMEOUT: {
            otErr << OT_METHOD << __FUNCTION__ << ": Reply timeout."
                  << std::endl;

            return {};
        } break;
        case SendResult::INVALID_REPLY: {
            sendStatus = true;
            otErr << OT_METHOD << __FUNCTION__ << ": Invalid reply."
                  << std::endl;

            return {};
        } break;
        case SendResult::VALID_REPLY: {
            sendStatus = true;
        } break;
        default: {
            otErr << OT_METHOD << __FUNCTION__ << ": Unknown error."
                  << std::endl;

            return {};
        }
    }

    OT_ASSERT(reply);

    const RequestNumber newNumber = reply->m_lNewRequestNum;
    Lock contextLock(lock_);
    add_acknowledged_number(contextLock, newNumber);
    remove_acknowledged_number(contextLock, *reply);
    request_number_.store(newNumber);
    update_remote_hash(contextLock, *reply);

    return newNumber;
}

// This is called by VerifyTransactionReceipt and VerifyBalanceReceipt.
//
// It's okay if some issued transaction #s in statement aren't found on the
// context, since the last balance agreement may have cleaned them out.
//
// But I should never see transaction numbers APPEAR in the context that aren't
// on the statement, since a balance agreement can ONLY remove numbers, not add
// them. So any numbers left over should still be accounted for on the last
// signed receipt.
//
// Conclusion: Loop through *this, which is newer, and make sure ALL numbers
// appear on the statement. No need to check the reverse, and no need to match
// the count.
bool ServerContext::Verify(const TransactionStatement& statement) const
{
    Lock lock(lock_);

    for (const auto& number : issued_transaction_numbers_) {
        const bool missing = (1 != statement.Issued().count(number));

        if (missing) {
            otOut << OT_METHOD << __FUNCTION__ << ": Issued transaction # "
                  << number << " on context not found on statement."
                  << std::endl;

            return false;
        }
    }

    // Getting here means that, though issued numbers may have been removed from
    // my responsibility in a subsequent balance agreement (since the
    // transaction agreement was signed), I know for a fact that no numbers have
    // been ADDED to my list of responsibility. That's the most we can verify
    // here, since we don't know the account number that was used for the last
    // balance agreement.

    return true;
}

bool ServerContext::VerifyTentativeNumber(const TransactionNumber& number) const
{
    return (0 < tentative_transaction_numbers_.count(number));
}
}  // namespace opentxs
