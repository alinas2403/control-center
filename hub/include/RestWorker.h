#ifndef CRESTWORKER_H
#define CRESTWORKER_H

#include <QObject>
#include <QString>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QSsl>
#include <QtNetwork/QSslConfiguration>
#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <vector>

class CSSEnvironment {
private:
  QString m_name;
public:
  CSSEnvironment() : m_name(""){}
  explicit CSSEnvironment(const QString& name) : m_name(name) {
  }
  ~CSSEnvironment(void){}
  const QString& name(void) const {return m_name;}
};
////////////////////////////////////////////////////////////////////////////

class CSSContainer {
private:
  std::map<QString, std::vector<QString> > m_dct_env_containers;
public:
  CSSContainer(){}
  explicit CSSContainer(const QJsonObject& json_obj) {
    for (int i = 0; i < json_obj.keys().count(); ++i) {
      QJsonArray arr = json_obj[json_obj.keys().at(i)].toArray();
      for (int j = 0; j < arr.size(); ++j) {
        m_dct_env_containers[json_obj.keys().at(i)].push_back(arr.at(j).toString());
      }
    }
  }
  explicit CSSContainer(const std::map<QString, std::vector<QString> >& dct) : m_dct_env_containers(dct) {}
  ~CSSContainer(void){}
  const std::map<QString, std::vector<QString> >& dct_env_containers() const {return m_dct_env_containers;}
  bool operator==(const CSSContainer& arg) {
    return m_dct_env_containers == arg.m_dct_env_containers;
  }
  bool operator!=(const CSSContainer& arg) {
    return !(this->operator ==(arg));
  }
};
////////////////////////////////////////////////////////////////////////////

class CSSPeerUser {
private:
  std::map<QString, std::vector<QString> > m_dct_peer_users;
public:
  CSSPeerUser(){}
  explicit CSSPeerUser(const QJsonObject& json_obj) {
    for (int i = 0; i < json_obj.keys().count(); ++i) {
      QJsonArray arr = json_obj[json_obj.keys().at(i)].toArray();
      for (int j = 0; j < arr.size(); ++j) {
        m_dct_peer_users[json_obj.keys().at(i)].push_back(arr.at(j).toString());
      }
    }
  }
  explicit CSSPeerUser(const std::map<QString, std::vector<QString> >& dct) : m_dct_peer_users(dct) {}
  ~CSSPeerUser() {}
  const std::map<QString, std::vector<QString> >& dct_peer_users() const {return m_dct_peer_users;}
  bool operator==(const CSSPeerUser& arg) {
    return m_dct_peer_users == arg.m_dct_peer_users;
  }
  bool operator!=(const CSSPeerUser& arg) {
    return !(this->operator ==(arg));
  }
};
////////////////////////////////////////////////////////////////////////////

class CSSBalance {
private:
  QString m_balance;
public:
  CSSBalance() : m_balance("") {}
  explicit CSSBalance(const QString& balance) : m_balance(balance) {
  }
  ~CSSBalance(){}

  const QString& value() const {return m_balance;}
};
////////////////////////////////////////////////////////////////////////////

class CRestWorker {
private:

  static QString m_id; //hack. will fixed on server part. we got it after login.

  static QByteArray send_request(const QNetworkRequest& req, bool get);
  static QByteArray get_request(const QNetworkRequest& req);
  static QByteArray post_request(const QNetworkRequest& req);
  static QJsonObject get_request_object(int& err_code, const QString& link);

  CRestWorker();
  CRestWorker(const QString& login,
              const QString& password);
  CRestWorker(const CRestWorker& worker);
  ~CRestWorker(void);
public:

  static int login(const QString& login, const QString& password);
  static std::vector<CSSEnvironment> get_environments(int& err_code);
  static CSSContainer get_containers(int& err_code);
  static CSSPeerUser get_peer_users(int& err_code);
  static CSSBalance get_balance(int& err_code);
};

#endif // CRESTWORKER_H
