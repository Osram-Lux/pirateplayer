#include "twitterfeedmodel.h"

#include <QUrl>
#include <QDomDocument>
#include <QNetworkReply>
#include <QtAlgorithms>
#include <QDebug>

TwitterFeedModel::TwitterFeedModel(QNetworkAccessManager *nam, QObject *parent) :
    QAbstractTableModel(parent)
{
    network = nam;
}

int TwitterFeedModel::rowCount(const QModelIndex &parent) const {
    return tweets.count();
}

int TwitterFeedModel::columnCount(const QModelIndex &parent) const {
    return COLUMN_COUNT;
}

QVariant TwitterFeedModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case TextColumn:
            return QVariant(tweets[index.row()].value("text", ""));
        case LinkColumn:
            return QVariant(tweets[index.row()].value("link", ""));
        case PubDateColumn:
            return QVariant(tweets[index.row()].value("pubdate", ""));
        }
    }

    return QVariant();
}

void TwitterFeedModel::setScreenName(QString name) {
    screenName = name;
    QUrl url = QUrl("http://twitter.com/statuses/user_timeline.rss");
    url.addQueryItem("screen_name", screenName);
    url.addQueryItem("count", "10");

    feed = network->get(QNetworkRequest(url));
    connect(feed, SIGNAL(finished()), this, SLOT(feedDownloaded()));
}

void TwitterFeedModel::feedDownloaded() {
    int offset = screenName.length() + 2;
    QDomDocument doc;
    doc.setContent(feed);
    QDomNodeList tweetList = doc.elementsByTagName("item");

    beginInsertRows(QModelIndex(), 0, tweetList.count()-1);
    for(int i = 0; i<tweetList.count(); i++) {
        QHash<QString,QString> s = QHash<QString,QString>();

        s.insert("text", tweetList.item(i).namedItem("description").toElement().text().remove(0, offset));
        s.insert("pubdate", tweetList.item(i).namedItem("pubDate").toElement().text());
        s.insert("link", tweetList.item(i).namedItem("link").toElement().text());

        tweets.append(s);
    }
    endInsertRows();
    emit dataChanged(createIndex(0, 0), createIndex(tweetList.count(), 2));

    feed->deleteLater();
}