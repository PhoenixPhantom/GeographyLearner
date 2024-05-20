#include "AcquireCredentialsWidget.h"
#include "git2/credential.h"
#include "git2/errors.h"
#include <qboxlayout.h>
#include <qcoreapplication.h>
#include <qgridlayout.h>
#include <qlabel.h>
#include <qnamespace.h>
#include <qpushbutton.h>

namespace CredentialsManager{
    int acquireCredentials(git_credential **out, [[maybe_unused]] const char *url, const char *username_from_url,
            unsigned int allowed_types, [[maybe_unused]] void *payload)
    {
        int error = 1;

        QString uname;
        if(username_from_url != nullptr) uname = QString(username_from_url);
        RequiredCredentials requiredType;

        if(allowed_types & GIT_CREDENTIAL_SSH_KEY) requiredType = SSH;
        else if(allowed_types & GIT_CREDENTIAL_USERPASS_PLAINTEXT) requiredType = Plaintext;
        else if(allowed_types & GIT_CREDENTIAL_USERNAME) requiredType = UsernameOnly;
        else return 0;

        if(!uname.isEmpty() && requiredType == UsernameOnly){
            error = git_credential_username_new(out, uname.toUtf8());
        }
        else{
            //AcquireCredsData* data = (AcquireCredsData*)payload;
            AcquireCredentialsWidget* credentialsAcquierer = new AcquireCredentialsWidget(requiredType, uname, nullptr);
            credentialsAcquierer->setWindowTitle("Zugriffseingabe");
            credentialsAcquierer->setWindowModality(Qt::NonModal);
            credentialsAcquierer->show();

            //wait for reaction
            while(credentialsAcquierer->isVisible() && !credentialsAcquierer->hasUserAccepted() &&
                    !credentialsAcquierer->hasUserDenied()){
                QCoreApplication::processEvents();
            }


            credentialsAcquierer->close();
            if(!credentialsAcquierer->hasUserAccepted()){
                delete credentialsAcquierer;
                return GIT_EUSER;
            }

            switch(requiredType){
                case SSH:
                    error = git_credential_ssh_key_new(out, credentialsAcquierer->getUsername().toUtf8(),
                            credentialsAcquierer->getSSHPubkey().toUtf8(),
                            credentialsAcquierer->getSSHPrivkey().toUtf8(),
                            credentialsAcquierer->getPassword().toUtf8());
                    break;
                case Plaintext:
                    error = git_credential_userpass_plaintext_new(out, credentialsAcquierer->getUsername().toUtf8(),
                            credentialsAcquierer->getPassword().toUtf8());
                    break;
                case UsernameOnly:
                    error = git_credential_username_new(out, credentialsAcquierer->getUsername().toUtf8());
                    break;
            }
            delete credentialsAcquierer;
        }

out:
        uname = "";
        return error;
    }


    AcquireCredentialsWidget::AcquireCredentialsWidget(RequiredCredentials requirements,
            const QString& givenUname, QWidget* parent) : QWidget(parent),
        sshPrivkey(nullptr), password(nullptr), wasAccepted(false), wasDenied(false)
    {
        QGridLayout* mainLayout = new QGridLayout(this);
        username = new QLineEdit(this);
        if(!givenUname.isEmpty()) {
            username->setEnabled(false);
            username->setText(givenUname);
        }
        mainLayout->addWidget(new QLabel(tr("Benutzername: ")), 0, 0);
        mainLayout->addWidget(username, 0, 1);
        QLineEdit** lastEdit = &username;

        int occupiedRows = 1;
        switch(requirements){
            case SSH:
                occupiedRows++;
                sshPrivkey = new QLineEdit(this);
                mainLayout->addWidget(new QLabel(tr("SSH Privatekey: ")), occupiedRows-1, 0);
                mainLayout->addWidget(sshPrivkey, occupiedRows-1, 1);

            case Plaintext:
                occupiedRows++;
                password = new QLineEdit(this);
                mainLayout->addWidget(new QLabel(tr("Passwort (falls vorhanden): ")), occupiedRows-1, 0);
                mainLayout->addWidget(password, occupiedRows-1, 1);
                lastEdit = &password;
                break;
            case UsernameOnly:
                break;
        }

        auto acceptLambda = [this](){ 
                if(!wasDenied && !username->text().isEmpty() &&
                        (sshPrivkey == nullptr || !sshPrivkey->text().isEmpty() )) wasAccepted = true;
        };

        connect(*lastEdit, &QLineEdit::returnPressed, this, acceptLambda);

        accept = new QPushButton(tr("Weiter"), this);
        connect(accept, &QPushButton::released, this, acceptLambda);
        mainLayout->addWidget(accept, occupiedRows, 0);

        deny = new QPushButton(tr("Abbrechen"), this);
        connect(deny, &QPushButton::released, this, [this](){ if(!wasAccepted) wasDenied = true;});
        mainLayout->addWidget(deny, occupiedRows, 1);

        setLayout(mainLayout);
    }

    AcquireCredentialsWidget::~AcquireCredentialsWidget(){ 
        username->setText("");
        delete username;
        if(sshPrivkey != nullptr){
            sshPrivkey->setText("");
            delete sshPrivkey;
        }
        if(password != nullptr){
            password->setText("");
            delete password;
        }
        delete accept;
        delete deny;
    }
}
