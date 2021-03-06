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

#ifndef OPENTXS_CORE_API_API_HPP
#define OPENTXS_CORE_API_API_HPP

#include "opentxs/Version.hpp"

#include <memory>
#include <mutex>
#include <string>

namespace opentxs
{

class CryptoEngine;
class MadeEasy;
class OT;
class OT_API;
class OT_ME;
class OTAPI_Exec;
class OTME_too;

namespace api
{

class Activity;
class ContactManager;
class Identity;
class Settings;
class Storage;
class Wallet;
class ZMQ;

class Api
{
public:
    std::recursive_mutex& Lock() const;

    OTAPI_Exec& Exec(const std::string& wallet = "");
    MadeEasy& ME(const std::string& wallet = "");
    OT_API& OTAPI(const std::string& wallet = "");
    OT_ME& OTME(const std::string& wallet = "");
    OTME_too& OTME_TOO(const std::string& wallet = "");

    ~Api() = default;

private:
    friend class opentxs::OT;

    Activity& activity_;
    Settings& config_;
    ContactManager& contacts_;
    CryptoEngine& crypto_engine_;
    Identity& identity_;
    Storage& storage_;
    Wallet& wallet_;
    ZMQ& zmq_;

    std::unique_ptr<OT_API> ot_api_;
    std::unique_ptr<OTAPI_Exec> otapi_exec_;
    std::unique_ptr<MadeEasy> made_easy_;
    std::unique_ptr<OT_ME> ot_me_;
    std::unique_ptr<OTME_too> otme_too_;

    mutable std::recursive_mutex lock_;

    void Cleanup();
    void Init();

    Api(Activity& activity,
        Settings& config,
        ContactManager& contacts,
        CryptoEngine& crypto,
        Identity& identity,
        Storage& storage,
        Wallet& wallet,
        ZMQ& zmq);
    Api() = delete;
    Api(const Api&) = delete;
    Api(Api&&) = delete;
    Api& operator=(const Api&) = delete;
    Api& operator=(Api&&) = delete;
};
}  // namespace api
}  // namespace opentxs

#endif  // OPENTXS_CORE_API_API_HPP
