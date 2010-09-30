/*
 * This file is part of Late.
 * Late is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Late is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with nosebus.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NEXTBUS_H
#define NEXTBUS_H

#include <QUrl>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QString>
#include <QXmlStreamReader>
#include <QDesktopServices>
#include <QDir>
#include <QEventLoop>

#include <QDebug>

#define INFO 1
#define CRITICAL 2

class StopData {
	public:
		QString tag;
		QString stopTag;
		QString title;
		double lat;
		double lon;
};

Q_DECLARE_METATYPE( StopData );
inline bool operator < ( const StopData &stop1, const StopData &stop2 );
QDataStream & operator << ( QDataStream & s, const StopData & t ); 
QDataStream & operator >> ( QDataStream & s, StopData & t );
bool operator == ( const StopData &a, const StopData &b );

class TitleTuple {
	public:
		QString tag;
		QString title;
		QString shortTitle;
};

Q_DECLARE_METATYPE( TitleTuple );
QDataStream & operator << ( QDataStream & s, const TitleTuple & t ); 
QDataStream & operator >> ( QDataStream & s, TitleTuple & t );
bool operator == ( const TitleTuple &a, const TitleTuple &b );

class PredData {
	public:
		QString minutes;
		QString seconds;
		QString vehicle;
		QString dirTag;
		QString dirTitle;

};

inline bool operator < ( const PredData &, const PredData & );

class NextBus : public QObject {
	Q_OBJECT
	public:
		explicit NextBus();
	

		/* Agency Functions */
		QList<TitleTuple> agencyList();
		int setAgency( TitleTuple agency );
		int setAgency( QString agencyTag, QString agencyTitle = "", QString agencyShortTitle = "" );
		TitleTuple getAgency();
		QString getAgencyTag();
		QString getAgencyTitle();
		QString getAgencyShortTitle();

		/* Route Functions */
		QList<TitleTuple> routeList();
		int setRoute( TitleTuple route );
		int setRoute( QString routeTag, QString routeTitle = "", QString routeShortTitle = "" );
		TitleTuple getRoute();
		QString getRouteTag();
		QString getRouteTitle();
		QString getRouteShortTitle();

		/* Direction Functions */
		QList<TitleTuple> directionList();
		int setDirection( TitleTuple direction );
		int setDirection( QString directionTag, QString directionTitle = "", QString directionShortTitle = "" );
		TitleTuple getDirection();
		QString getDirectionTag();
		QString getDirectionTitle();
		QString getDirectionShortTitle();

		/* Stop Functions */
		QList<StopData> stopList();
		StopData getStop( QString stopTag );
		QString getStopTitle( QString stopTag );

		QList<PredData> getPredictionsSimple( QString *message, QString tag );

	private:
		QString dataDir;

		/* Network Stuffs */
		QUrl baseUrl;
		QNetworkAccessManager netMan;
		QNetworkRequest httpReq;

		/* Agencies */
		TitleTuple curAgency;

		/* Routes */
		TitleTuple curRoute;
		int _routeList();
		int _routeInfo( QString routeTag = "" );//loads directions, stops for route

		/* Directions */
		TitleTuple curDirection;

		/* Stops */
		QMap<QString,StopData> listStops;
		int _listStops();
		StopData curStop;

	signals:
		void downloadProgress( qint64 bytesReceived, qint64 bytesTotal );
		void notice( int type, QString message );

	public slots:
		void netDownloadProgress( qint64 bytesReceived, qint64 bytesTotal );

};

#endif // NEXTBUS_H
