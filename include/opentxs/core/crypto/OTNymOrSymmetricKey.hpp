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

#ifndef OPENTXS_CORE_CRYPTO_OTNYMORSYMMETRICKEY_HPP
#define OPENTXS_CORE_CRYPTO_OTNYMORSYMMETRICKEY_HPP

#include "opentxs/Version.hpp"

namespace opentxs
{

class OTEnvelope;
class Identifier;
class OTPassword;
class Nym;
class String;
class OTSymmetricKey;

// There are certain cases where we want the option to pass a Nym OR a
// symmetric key, and the function should be able to handle either.
// This class is used to make that possible.
//
class OTNym_or_SymmetricKey
{
private:
    Nym* m_pNym;

    OTSymmetricKey* m_pKey{nullptr};
    OTPassword* m_pPassword{nullptr};  // optional. Goes with m_pKey.

    // m_pPassword is usually not owned. But if we create it and keep it around
    // to avoid
    // (for example forcing the user to enter the PW 30 times in a row when
    // exporting his purse...)
    // then we want to set this to true (where it normally defaults to false) in
    // order to make sure we cleanup on destruction.
    bool m_bCleanupPassword{false};

    const String* m_pstrDisplay{nullptr};

    OTNym_or_SymmetricKey();

public:
    EXPORT Nym* GetNym() const { return m_pNym; }
    EXPORT OTSymmetricKey* GetKey() const { return m_pKey; }
    EXPORT OTPassword* GetPassword() const
    {
        return m_pPassword;
    }  // for symmetric key (optional)

    EXPORT bool IsNym() const { return (nullptr != m_pNym); }
    EXPORT bool IsKey() const { return (nullptr != m_pKey); }
    EXPORT bool HasPassword() const
    {
        return (nullptr != m_pPassword);
    }  // for symmetric key (optional)

    EXPORT void GetIdentifier(Identifier& theIdentifier) const;
    EXPORT void GetIdentifier(String& strIdentifier) const;

    EXPORT bool CompareID(const OTNym_or_SymmetricKey& rhs) const;

    // Seal / Open is for public / private key crypto. (With OTPseudonym and
    // OTAsymmetricKey.)
    // Whereas Encrypt/Decrypt is for symmetric key crypto (With
    // OTSymmetricKey.)
    EXPORT bool Seal_or_Encrypt(
        OTEnvelope& outputEnvelope,
        const String& strInput,
        const String* pstrDisplay = nullptr);
    EXPORT bool Open_or_Decrypt(
        const OTEnvelope& inputEnvelope,
        String& strOutput,
        const String* pstrDisplay = nullptr);

    EXPORT ~OTNym_or_SymmetricKey();

    EXPORT OTNym_or_SymmetricKey(const OTNym_or_SymmetricKey& rhs);

    EXPORT OTNym_or_SymmetricKey(
        const Nym& theNym,
        const String* pstrDisplay = nullptr);
    EXPORT OTNym_or_SymmetricKey(
        const OTSymmetricKey& theKey,
        const String* pstrDisplay = nullptr);
    EXPORT OTNym_or_SymmetricKey(
        const OTSymmetricKey& theKey,
        const OTPassword& thePassword,
        const String* pstrDisplay = nullptr);

    EXPORT void swap(OTNym_or_SymmetricKey& other);

    EXPORT OTNym_or_SymmetricKey& operator=(
        OTNym_or_SymmetricKey other);  // passed by value.

    EXPORT void Release();  // Someday make this virtual, if we ever subclass
                            // it.
    EXPORT void Release_Nym_or_SymmetricKey();  // NOT called in the destructor,
                                                // since this normally doesn't
                                                // own its contents.
};

}  // namespace opentxs

#endif  // OPENTXS_CORE_CRYPTO_OTNYMORSYMMETRICKEY_HPP
