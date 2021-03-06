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

#include "opentxs/server/Transactor.hpp"

#include "opentxs/api/OT.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/cash/Mint.hpp"
#include "opentxs/consensus/ClientContext.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/AccountList.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/OTFolders.hpp"
#include "opentxs/server/MainFile.hpp"
#include "opentxs/server/Server.hpp"

#include <inttypes.h>
#include <stdint.h>
#include <map>
#include <memory>
#include <string>
#include <utility>

namespace opentxs::server
{

Transactor::Transactor(Server* server)
    : transactionNumber_(0)
    , server_(server)
{
}

Transactor::~Transactor() {}

/// Just as every request must be accompanied by a request number, so
/// every transaction request must be accompanied by a transaction number.
/// The request numbers can simply be incremented on both sides (per user.)
/// But the transaction numbers must be issued by the server and they do
/// not repeat from user to user. They are unique to transaction.
///
/// Users must ask the server to send them transaction numbers so that they
/// can be used in transaction requests.
bool Transactor::issueNextTransactionNumber(
    TransactionNumber& lTransactionNumber)
{
    // transactionNumber_ stores the last VALID AND ISSUED transaction number.
    // So first, we increment that, since we don't want to issue the same number
    // twice.
    transactionNumber_++;

    // Next, we save it to file.
    if (!server_->mainFile_.SaveMainFile()) {
        Log::Error("Error saving main server file.\n");
        transactionNumber_--;
        return false;
    }

    // SUCCESS?
    // Now the server main file has saved the latest transaction number,
    // NOW we set it onto the parameter and return true.
    lTransactionNumber = transactionNumber_;
    return true;
}

bool Transactor::issueNextTransactionNumberToNym(
    ClientContext& context,
    TransactionNumber& lTransactionNumber)
{
    if (!issueNextTransactionNumber(lTransactionNumber)) {
        return false;
    }

    // Each Nym stores the transaction numbers that have been issued to it.
    // (On client AND server side.)
    //
    // So whenever the server issues a new number, it's to a specific Nym, then
    // it is recorded in his Nym file before being sent to the client (where it
    // is also recorded in his Nym file.)  That way the server always knows
    // which numbers are valid for each Nym.
    if (!context.IssueNumber(transactionNumber_)) {
        Log::Error("Error adding transaction number to Nym file.\n");
        transactionNumber_--;
        // Save it back how it was, since we're not issuing this number after
        // all.
        server_->mainFile_.SaveMainFile();

        return false;
    }

    // SUCCESS?
    // Now the server main file has saved the latest transaction number,
    // NOW we set it onto the parameter and return true.
    lTransactionNumber = transactionNumber_;

    return true;
}

// Server stores a map of BASKET_ID to BASKET_ACCOUNT_ID.
bool Transactor::addBasketAccountID(
    const Identifier& BASKET_ID,
    const Identifier& BASKET_ACCOUNT_ID,
    const Identifier& BASKET_CONTRACT_ID)
{
    Identifier theBasketAcctID;

    if (lookupBasketAccountID(BASKET_ID, theBasketAcctID)) {
        Log::Output(0, "User attempted to add Basket that already exists.\n");
        return false;
    }

    String strBasketID(BASKET_ID), strBasketAcctID(BASKET_ACCOUNT_ID),
        strBasketContractID(BASKET_CONTRACT_ID);

    idToBasketMap_[strBasketID.Get()] = strBasketAcctID.Get();
    contractIdToBasketAccountId_[strBasketContractID.Get()] =
        strBasketAcctID.Get();

    return true;
}

/// Use this to find the basket account ID for this server (which is unique to
/// this server)
/// using the contract ID to look it up. (The basket contract ID is unique to
/// this server.)
bool Transactor::lookupBasketAccountIDByContractID(
    const Identifier& BASKET_CONTRACT_ID,
    Identifier& BASKET_ACCOUNT_ID)
{
    // Server stores a map of BASKET_ID to BASKET_ACCOUNT_ID. Let's iterate
    // through that map...
    for (auto& it : contractIdToBasketAccountId_) {
        String strBasketContractID = it.first.c_str();
        String strBasketAcctID = it.second.c_str();

        Identifier id_BASKET_CONTRACT(strBasketContractID),
            id_BASKET_ACCT(strBasketAcctID);

        if (BASKET_CONTRACT_ID == id_BASKET_CONTRACT)  // if the basket contract
                                                       // ID passed in matches
                                                       // this one...
        {
            BASKET_ACCOUNT_ID = id_BASKET_ACCT;
            return true;
        }
    }
    return false;
}

/// Use this to find the basket account ID for this server (which is unique to
/// this server)
/// using the contract ID to look it up. (The basket contract ID is unique to
/// this server.)
bool Transactor::lookupBasketContractIDByAccountID(
    const Identifier& BASKET_ACCOUNT_ID,
    Identifier& BASKET_CONTRACT_ID)
{
    // Server stores a map of BASKET_ID to BASKET_ACCOUNT_ID. Let's iterate
    // through that map...
    for (auto& it : contractIdToBasketAccountId_) {
        String strBasketContractID = it.first.c_str();
        String strBasketAcctID = it.second.c_str();

        Identifier id_BASKET_CONTRACT(strBasketContractID),
            id_BASKET_ACCT(strBasketAcctID);

        if (BASKET_ACCOUNT_ID == id_BASKET_ACCT)  // if the basket contract ID
                                                  // passed in matches this
                                                  // one...
        {
            BASKET_CONTRACT_ID = id_BASKET_CONTRACT;
            return true;
        }
    }
    return false;
}

/// Use this to find the basket account for this server (which is unique to this
/// server)
/// using the basket ID to look it up (the Basket ID is the same for all
/// servers)
bool Transactor::lookupBasketAccountID(
    const Identifier& BASKET_ID,
    Identifier& BASKET_ACCOUNT_ID)
{
    // Server stores a map of BASKET_ID to BASKET_ACCOUNT_ID. Let's iterate
    // through that map...
    for (auto& it : idToBasketMap_) {
        String strBasketID = it.first.c_str();
        String strBasketAcctID = it.second.c_str();

        Identifier id_BASKET(strBasketID), id_BASKET_ACCT(strBasketAcctID);

        if (BASKET_ID ==
            id_BASKET)  // if the basket ID passed in matches this one...
        {
            BASKET_ACCOUNT_ID = id_BASKET_ACCT;
            return true;
        }
    }
    return false;
}

/// Looked up the voucher account (where cashier's cheques are issued for any
/// given instrument definition) return a pointer to the account.  Since it's
/// SUPPOSED to
/// exist, and since it's being requested, also will GENERATE it if it cannot
/// be found, add it to the list, and return the pointer. Should always succeed.
std::shared_ptr<Account> Transactor::getVoucherAccount(
    const Identifier& INSTRUMENT_DEFINITION_ID)
{
    std::shared_ptr<Account> pAccount;
    const Identifier NOTARY_NYM_ID(server_->m_nymServer),
        NOTARY_ID(server_->m_strNotaryID);
    bool bWasAcctCreated = false;
    pAccount = voucherAccounts_.GetOrRegisterAccount(
        server_->m_nymServer,
        NOTARY_NYM_ID,
        INSTRUMENT_DEFINITION_ID,
        NOTARY_ID,
        bWasAcctCreated);
    if (bWasAcctCreated) {
        String strAcctID;
        pAccount->GetIdentifier(strAcctID);
        const String strInstrumentDefinitionID(INSTRUMENT_DEFINITION_ID);

        Log::vOutput(
            0,
            "Server::GetVoucherAccount: Successfully created "
            "voucher account ID: %s Instrument Definition ID: %s\n",
            strAcctID.Get(),
            strInstrumentDefinitionID.Get());

        if (!server_->mainFile_.SaveMainFile()) {
            Log::Error("Server::GetVoucherAccount: Error saving main "
                       "server file containing new account ID!!\n");
        }
    }

    return pAccount;
}
}  // namespace opentxs::server
