#ifndef ACQUIRECREDENTIALS_WIDGET_H
#define ACQUIRECREDENTIALS_WIDGET_H

#include "git2/credential.h"
#include <cstdint>
#include <qlineedit.h>
#include <qwidget.h>

class QPushButton;

namespace CredentialsManager{
    int acquireCredentials(git_credential **out, [[maybe_unused]] const char *url, const char *username_from_url,
            unsigned int allowed_types, [[maybe_unused]] void *payload);

    enum RequiredCredentials : uint8_t{
        SSH,
        Plaintext,
        UsernameOnly
    };


    class AcquireCredentialsWidget : public QWidget{
        Q_OBJECT

        private:
            friend int acquireCredentials(git_credential **out, const char *url, const char *username_from_url, unsigned int allowed_types, void *payload);

            QLineEdit* username;
            QLineEdit* sshPrivkey;
            QLineEdit* password;
            QPushButton* accept;
            QPushButton* deny;
            bool wasAccepted;
            bool wasDenied;

            explicit AcquireCredentialsWidget(RequiredCredentials requirements, const QString& givenUname, QWidget* parent);
            ~AcquireCredentialsWidget();

            bool hasUserAccepted() { return wasAccepted; };
            bool hasUserDenied() { return wasDenied; };

            QString getUsername() const { return username->text();} 
            QString getSSHPubkey() const { return sshPrivkey == nullptr ? "" : QString("%s.pub").arg(sshPrivkey->text());}
            QString getSSHPrivkey() const { return sshPrivkey == nullptr ? "" : sshPrivkey->text();}
            QString getPassword() const { return password == nullptr ? "" : password->text();}

    };
}

#endif //ACQUIRECREDENTIALS_WIDGET_H
