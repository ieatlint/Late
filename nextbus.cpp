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

#include "nextbus.h"


QDataStream & operator << ( QDataStream & s, const TitleTuple & t ) {
	s << t.tag;
	s << t.title;
	s << t.shortTitle;
	return s;
}
QDataStream & operator >> ( QDataStream & s, TitleTuple & t ) {
	s >> t.tag;
	s >> t.title;
	s >> t.shortTitle;
	return s;
}

bool operator == ( const TitleTuple &a, const TitleTuple &b ) {
	if( a.tag == b.tag )
		return true;
	return false;
}


QDataStream & operator << ( QDataStream & s, const StopData & t ) {
	s << t.tag;
	s << t.stopTag;
	s << t.title;
	s << t.lat;
	s << t.lon;
	return s;
}

QDataStream & operator >> ( QDataStream & s, StopData & t ) {
	s >> t.tag;
	s >> t.stopTag;
	s >> t.title;
	s >> t.lat;
	s >> t.lon;
	return s;
}

inline bool operator < ( const StopData &stop1, const StopData &stop2 ) {
	return stop1.tag < stop2.tag;
}

bool operator == ( const StopData &a, const StopData &b ) {
	if( a.tag == b.tag )
		return true;
	return false;
}


inline bool operator < ( const PredData &a, const PredData &b ) {
	return a.seconds.toInt() < b.seconds.toInt();
}

NextBus::NextBus() {

	dataDir = QDesktopServices::storageLocation( QDesktopServices::DataLocation );
	dataDir.append( "/" );

	QDir dir;
	if( !dir.exists( dataDir ) && !dir.mkpath( dataDir ) ) {
		qWarning() << "Failed to make data dir:" << dataDir;
		emit notice( CRITICAL, "Failed to make data dir:\n" + dataDir );
	}

	baseUrl.setScheme( "http" );
	baseUrl.setHost( "webservices.nextbus.com" );
	baseUrl.setPath( "/service/publicXMLFeed" );

	httpReq.setRawHeader( "User-Agent", "late" );
}

/* Agency Functions */

int NextBus::setAgency( TitleTuple agency ) {
	curAgency = agency;
	return 0;
}

int NextBus::setAgency( QString agencyTag, QString agencyTitle, QString agencyShortTitle ) {
	if( agencyTitle.isEmpty() ) {
		QList<TitleTuple> agencies = agencyList();
		int i;
		for( i = 0; i < agencies.length(); i++ ) {
			TitleTuple agency = agencies.at( i );
			if( agency.tag == agencyTag ) {
				curAgency = agency;
				break;
			}
		}
		if( i == agencies.length() ) {
			qWarning() << "Unable to set agency to" << agencyTag << " -- not found in list!";
			return 1;
		}

	} else {
		curAgency.tag = agencyTag;
		curAgency.title = agencyTitle;
		curAgency.shortTitle = agencyShortTitle;
	}

	return 0;
}

TitleTuple NextBus::getAgency() {
	return curAgency;
}

QString NextBus::getAgencyTitle() {
	return curAgency.title;
}

QString NextBus::getAgencyTag() {
	return curAgency.tag;
}

QString NextBus::getAgencyShortTitle() {
	if( !curAgency.shortTitle.isEmpty() )
		return curAgency.shortTitle;
	else
		return curAgency.title;
}

QList<TitleTuple> NextBus::agencyList() {
	QList<TitleTuple> agencies;
	TitleTuple agency;

	agency.title = "AC Transit";
	agency.tag = "actransit";
	agencies.append( agency );

	agency.tag = "calu-pa";
	agency.title = "California University of Pennsylvania";
	agencies.append( agency );

	agency.tag = "charm-city";
	agency.title = "Charm City Circulator";
	agencies.append( agency );

	agency.tag = "mbta";
	agency.title = "MBTA";
	agencies.append( agency );

	agency.tag = "mit";
	agency.title = "Massachusetts Institute of Technology";
	agencies.append( agency );

	agency.tag = "portland-sc";
	agency.title = "Portland Streetcar";
	agencies.append( agency );

	agency.tag = "rutgers";
	agency.title = "Rutgers University";
	agencies.append( agency );

	agency.tag = "rutgers-newark";
	agency.title = "Rutgers-Newark College Town Shuttle";
	agencies.append( agency );

	agency.tag = "sf-muni";
	agency.title = "San Francisco MUNI";
	agencies.append( agency );

	return agencies;
}

/*** Route Functions ***/

int NextBus::setRoute( TitleTuple route ) {
	curRoute = route;
	return 0;
}

int NextBus::setRoute( QString routeTag, QString routeTitle, QString routeShortTitle ) {
	if( routeTitle.isEmpty() ) {
		QList<TitleTuple> routes = routeList();
		int i;
		for( i = 0; i < routes.length(); i++ ) {
			TitleTuple route = routes.at( i );
			if( route.tag == routeTag ) {
				curRoute = route;
				break;
			}
		}
		if( i == routes.length() ) {
			qWarning() << "Unable to set route to" << routeTag << " -- not found in list!";
			return 1;
		}
	} else {
		curRoute.tag = routeTag;
		curRoute.title = routeTitle;
		curRoute.shortTitle = routeShortTitle;
	}

	return 0;
}

TitleTuple NextBus::getRoute() {
	return curRoute;
}

QString NextBus::getRouteTag() {
	return curRoute.tag;
}

QString NextBus::getRouteTitle() {
	return curRoute.title;
}

QString NextBus::getRouteShortTitle() {
	if( curRoute.shortTitle.isEmpty() )
		return curRoute.title;
	else
		return curRoute.shortTitle;
}

QList<TitleTuple> NextBus::routeList() {
	QFile routeFile;
	QList<TitleTuple> routes;

	routeFile.setFileName( dataDir + curAgency.tag + "/routes" );
	if( !routeFile.exists() && _routeList() ) {
		qWarning() << "Failed to get routes";
		return routes;
	}
	
	if( !routeFile.open( QIODevice::ReadOnly ) ) {
		qWarning() << "Failed to open routes file for reading";
		emit notice( CRITICAL, "Unable to open data, check permissions" );
		return routes;
	}
	QDataStream in( &routeFile );
	in >> routes;

	routeFile.close();

	return routes;
}

int NextBus::_routeList() {
	qDebug() << "Fetching routes from nextbus.com for" << curAgency.tag;
	QUrl url = baseUrl;

	url.addQueryItem( "command", "routeList" );
	url.addQueryItem( "a", curAgency.tag  );

	httpReq.setUrl( url );

	QNetworkReply *reply;
	reply = netMan.get( httpReq );

	QEventLoop blockLoop;
	connect( &netMan, SIGNAL( finished( QNetworkReply * ) ), &blockLoop, SLOT( quit() ) );
	blockLoop.exec();

	if( reply->error() ) {
		qWarning() << "Aborted routeList fetch due to network error:" << reply->errorString();
		QString msg = "A network error occured:\n";
		msg.append( reply->errorString() );
		msg.append( "\n\n\n" );
		emit notice( CRITICAL, msg );
		return 2;
	}

	int responseCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
	if( responseCode != 200 ) {
		QString errorMsg;
		errorMsg.append( "Recieved response code %1 from nextbus.com." ).arg( responseCode );
		emit notice( CRITICAL, errorMsg );
		return 2;
	}

	QXmlStreamReader xml( reply->readAll() );
	QList<TitleTuple> routes;
	TitleTuple route;

	while( !xml.atEnd() ) {
		xml.readNext();

		if( xml.error() ) {
			qDebug() << "error:" << xml.errorString();
			return 1;
		}

		if( xml.isStartElement() ) {
			if( xml.name() == "route" ) {
				QXmlStreamAttributes attributes = xml.attributes();
				route.tag = attributes.value( "tag" ).toString();
				route.title = attributes.value( "title" ).toString();
				route.shortTitle = attributes.value( "shortTitle" ).toString();
				if( route.shortTitle.isEmpty() )
					route.shortTitle = route.tag;
				routes.append( route );
			} else if( xml.name() == "Error" ) {
				QString retry = xml.attributes().value( "shouldRetry" ).toString();
				QString error = xml.readElementText();
				QString errorMsg = "NextBus.com Error:\n" + error + "\n";
				if( retry == "true" )
					errorMsg.append( "This is a NextBus-side error.  Retry your request" );
				else
					errorMsg.append( "This is a client-side error.  Retrying will not fix this.  Please report this bug." );
				emit notice( CRITICAL, errorMsg );
				return 3;
				
			}
		}

	}

	qDebug() << "There are" << routes.size() << "routes";

	QDir dir;
	if( !dir.exists( dataDir + curAgency.tag ) && !dir.mkpath( dataDir + curAgency.tag ) ) {
		qWarning() << "Failed to make data dir:" << dataDir;
		emit notice( CRITICAL, "Unable to create data directory, check permissions" );
	}
	
	if( routes.length() < 1 ) {
		qWarning() << "Loaded no routes.  Assuming error occured.";
		emit notice( CRITICAL, "Failed to download list of routes.  Please check network connection and retry" );
		return 1;
	}

	QFile routeFile;
	routeFile.setFileName( dataDir + curAgency.tag + "/routes" );
	if( !routeFile.open( QIODevice::WriteOnly ) ) {
		qWarning() << "Failed to open routes file for writing";
		emit notice( CRITICAL, "Unable to create data file, check permissions" );
		return 1;
	}
	QDataStream out( &routeFile );
	out << routes;

	routeFile.close();

	return 0;
}

int  NextBus::_routeInfo( QString routeTag ) {
	QUrl url = baseUrl;

	qDebug() << "Fetching routeInfo from nextbus.com for" << curAgency.tag << routeTag;

	url.addQueryItem( "command", "routeConfig" );
	url.addQueryItem( "a", curAgency.tag  );
	if( !routeTag.isEmpty() )
		url.addQueryItem( "r", routeTag );

	
	httpReq.setUrl( url );

	QNetworkReply *reply;
	reply = netMan.get( httpReq );
	connect( reply, SIGNAL( downloadProgress( qint64, qint64 ) ), this, SLOT( netDownloadProgress( qint64, qint64 ) ) );

	QEventLoop blockLoop;
	connect( &netMan, SIGNAL( finished( QNetworkReply * ) ), &blockLoop, SLOT( quit() ) );
	blockLoop.exec();

	if( reply->error() ) {
		qWarning() << "Aborted routeInfo fetch due to network error:" << reply->errorString();
		QString msg = "A network error occured:\n";
		msg.append( reply->errorString() );
		msg.append( "\n\n\n" );
		emit notice( CRITICAL, msg );
		return 2;
	}
	int responseCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
	if( responseCode != 200 ) {
		QString errorMsg;
		errorMsg.append( "Recieved response code %1 from nextbus.com." ).arg( responseCode );
		emit notice( CRITICAL, errorMsg );
		return 2;
	}

	QXmlStreamReader xml( reply->readAll() );
	QString route;
	QList<TitleTuple> directions;
	TitleTuple direction;
	QStringList dirStops;
	StopData stop;

	if( listStops.isEmpty() )
		_listStops();

	while( !xml.atEnd() ) {
		xml.readNext();

		if( xml.error() ) {
			qDebug() << "error:" << xml.errorString();
			return 1;
		}

		if( xml.isStartElement() ) {
			if( xml.name() == "route" ) {
//				QXmlStreamAttributes attributes = xml.attributes();
				route = xml.attributes().value( "tag" ).toString();
			} else if( xml.name() == "stop" ) {
				QXmlStreamAttributes attributes = xml.attributes();
				if( attributes.hasAttribute( "title" ) ) {
					stop.tag = attributes.value( "tag" ).toString();
					stop.stopTag = attributes.value( "stopTag" ).toString();
					stop.title = attributes.value( "title" ).toString();
					QString tmp = attributes.value( "lat" ).toString();
					stop.lat = tmp.toDouble();
					tmp = attributes.value( "lon" ).toString();
					stop.lon = tmp.toDouble();

					if( !listStops.contains( stop.tag ) )
						listStops.insert( stop.tag, stop );

				} else if( !direction.tag.isEmpty() ) {
					dirStops << attributes.value( "tag" ).toString();
				}
			} else if( xml.name() == "direction" ) {
				QXmlStreamAttributes attributes = xml.attributes();
				if( attributes.value( "useForUI" ) == "true" ) {
					direction.tag = attributes.value( "tag" ).toString();
					direction.title = attributes.value( "title" ).toString();
					direction.shortTitle = attributes.value( "name" ).toString();

					directions.append( direction );
				}
			} else if( xml.name() == "Error" ) {
				QString retry = xml.attributes().value( "shouldRetry" ).toString();
				QString error = xml.readElementText().trimmed();
				QString errorMsg = "NextBus.com Error:\n" + error + "\n";
				if( retry == "true" )
					errorMsg.append( "This is a NextBus-side error.  Retry your request" );
				else
					errorMsg.append( "This is a client-side error.  Retrying will not fix this.  Please report this bug." );
				emit notice( CRITICAL, errorMsg );
				return 3;
			}
		} else if( xml.isEndElement() ) {
			if( xml.name() == "route" ) {
				if( !directions.isEmpty() ) {
					QDir dir;
					if( !dir.exists( dataDir + curAgency.tag + "/" + route ) && !dir.mkpath( dataDir + curAgency.tag + "/" + route ) ) {
						qWarning() << "Failed to make route dir:" << dataDir + curAgency.tag + "/" + route;
						emit notice( CRITICAL, "Unable to create data dir, check permissions" );
						return 1;
					}
					if( directions.length() < 1 ) {
						qWarning() << "Loaded no directions.  Assuming error occured.";
						emit notice( CRITICAL, "Failed to download list of route directions.  Please check network connection and retry" );
						continue;
					}

					QFile dirFile;
					dirFile.setFileName( dataDir + curAgency.tag + "/" + route + "/directions" );
					if( !dirFile.open( QIODevice::WriteOnly ) ) {
						qWarning() << "Failed to open directions file for writing";
						emit notice( CRITICAL, "Unable to create data file, check permissions" );
						return 1;
					}
					QDataStream dirOut( &dirFile );
					dirOut << directions;

					dirFile.close();

					directions.clear();
				}
				route = "";
			} else if( xml.name() == "direction" ) {
				if( !direction.tag.isEmpty() ) {
					QDir dir;
					if( !dir.exists( dataDir + curAgency.tag + "/" + route ) && !dir.mkpath( dataDir + curAgency.tag + "/" + route ) ) {
						qWarning() << "Failed to make route dir:" << dataDir + curAgency.tag + "/" + route;
						emit notice( CRITICAL, "Unable to create data dir, check permissions" );
						return 1;
					}

					if( dirStops.length() < 1 ) {
						qWarning() << "Loaded no directions.  Assuming error occured.";
						continue;
					}

					QFile dirFile;
					dirFile.setFileName( dataDir + curAgency.tag + "/" + route + "/" + direction.tag );
					if( !dirFile.open( QIODevice::WriteOnly ) ) {
						qWarning() << "Failed to open direction file for writing";
						return 1;
					}
					QDataStream dirOut( &dirFile );
					dirOut << dirStops;

					dirFile.close();
				}

				direction.tag = "";
				dirStops.clear();
			}
		}

	}
	/* serialise listStops */
	QDir dir;
	if( !dir.exists( dataDir + curAgency.tag ) && !dir.mkpath( dataDir + curAgency.tag ) )
		qWarning() << "Failed to make agency dir:" << dataDir + curAgency.tag;
	
	if( listStops.count() < 1 ) {
		qWarning() << "Loaded no stops.  Assuming error occured.";
		return 1;
	}

	
	QFile stopFile;
	stopFile.setFileName( dataDir + curAgency.tag + "/stops" );
	if( !stopFile.open( QIODevice::WriteOnly ) ) {
		qWarning() << "Failed to open stops file for writing";
		return 1;
	}
	QDataStream stopsOut( &stopFile );
	stopsOut << listStops;
	
	qDebug() << "Wrote stops:" << listStops.count();

	stopFile.close();

	return 0;
}

void NextBus::netDownloadProgress( qint64 bytesReceived, qint64 bytesTotal ) {
	emit downloadProgress( bytesReceived, bytesTotal );
}

/*** Direction Functions ***/

int NextBus::setDirection( TitleTuple direction ) {
	curDirection = direction;
	return 0;
}

int NextBus::setDirection( QString directionTag, QString directionTitle, QString directionShortTitle ) {
	if( directionTitle.isEmpty() ) {
		QList<TitleTuple> directions = directionList();
		int i;
		for( i = 0; i < directions.length(); i++ ) {
			TitleTuple direction = directions.at( i );
			if( direction.tag == directionTag ) {
				curDirection = direction;
				break;
			}
		}
		if( i == directions.length() ) {
			qWarning() << "Unable to set direction to" << directionTag << " -- not found in list!";
			return 1;
		}
	} else {
		curDirection.tag = directionTag;
		curDirection.title = directionTitle;
		curDirection.shortTitle = directionShortTitle;
	}

	return 0;
}

TitleTuple NextBus::getDirection() {
	return curDirection;
}

QString NextBus::getDirectionTag() {
	return curDirection.tag;
}

QString NextBus::getDirectionTitle() {
	return curDirection.title;
}

QString NextBus::getDirectionShortTitle() {
	if( curDirection.shortTitle.isEmpty() )
		return curDirection.title;
	else
		return curDirection.shortTitle;
}


QList<TitleTuple> NextBus::directionList() {
	QFile directionFile;
	QList<TitleTuple> directions;

	directionFile.setFileName( dataDir + curAgency.tag + "/" + curRoute.tag + "/directions" );
	if( !directionFile.exists() && _routeInfo( curRoute.tag ) ) {
		qWarning() << "Failed to get info for route";
		return directions;
	}
	
	if( !directionFile.open( QIODevice::ReadOnly ) ) {
		qWarning() << "Failed to open directions file for reading";
		return directions;
	}
	QDataStream in( &directionFile );
	in >> directions;

	directionFile.close();

	return directions;
}


/*** Stop Functions ***/
int NextBus::_listStops() {
	QFile stopFile;
	
	QString stopFileName = dataDir + curAgency.tag + "/stops";
	stopFile.setFileName( stopFileName );
	if( !stopFile.exists() && _routeInfo( curRoute.tag ) ) {
		qWarning() << "Failed to get stop list";
		return 1;
	}

	if( !stopFile.open( QIODevice::ReadOnly ) ) {
		qWarning() << "Failed to open stops file for reading";
		return 1;
	}

	QDataStream in( &stopFile );
	in >> listStops;

	stopFile.close();

	qDebug() << "Loaded stop:" << listStops.count();

	return 0;
}
QList<StopData> NextBus::stopList() {
	QList<StopData> stops;

	QFile dirFile;

	QString dirFileName = dataDir + curAgency.tag + "/" + curRoute.tag + "/" + curDirection.tag;
	dirFile.setFileName( dirFileName );
	if( !dirFile.exists() && _routeInfo( curRoute.tag ) ) {
		qWarning() << "Failed to get info for directon";
		return stops;
	}

	if( !dirFile.open( QIODevice::ReadOnly ) ) {
		qWarning() << "Failed to open direction file for reading";
		return stops;
	}
	QDataStream in( &dirFile );
	QStringList dirStops;
	in >> dirStops;

	dirFile.close();

	if( listStops.isEmpty() )
		_listStops();
	
	for( int i = 0; i < dirStops.length(); i++ ) {
		stops.append( listStops.value( dirStops.at( i ) ) );
	}

	return stops;
}

StopData NextBus::getStop( QString stopTag ) {
	return listStops.value( stopTag );
}

QString NextBus::getStopTitle( QString stopTag ) {
	StopData stop = getStop( stopTag );
	return stop.title;
}


QList<PredData> NextBus::getPredictionsSimple( QString *message, QString tag ) {
	qDebug() << "Fetching simple prediction from nextbus.com for" << curAgency.tag << curRoute.tag << tag;
	QList<PredData> predictions;
	PredData prediction;
	QUrl url = baseUrl;

	url.addQueryItem( "command", "predictions" );
	url.addQueryItem( "a", curAgency.tag  );
	url.addQueryItem( "r", curRoute.tag );
	url.addQueryItem( "s", tag );

	httpReq.setUrl( url );

	QNetworkReply *reply;
	reply = netMan.get( httpReq );

	QEventLoop blockLoop;
	connect( &netMan, SIGNAL( finished( QNetworkReply * ) ), &blockLoop, SLOT( quit() ) );
	blockLoop.exec();

	if( reply->error() ) {
		qWarning() << "Aborted simple prediction fetch due to network error:" << reply->errorString();
		QString msg = "A network error occured:\n";
		msg.append( reply->errorString() );
		msg.append( "\n\n\n" );
		emit notice( CRITICAL, msg );
		return predictions;
	}

	int responseCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
	if( responseCode != 200 ) {
		QString errorMsg;
		errorMsg.append( "Recieved response code %1 from nextbus.com." ).arg( responseCode );
		emit notice( CRITICAL, errorMsg );
		return predictions;
	}

	QXmlStreamReader xml( reply->readAll() );

	message->clear();

	while( !xml.atEnd() ) {
		xml.readNext();

		if( xml.error() ) {
			qDebug() << "error:" << xml.errorString();
			return predictions;
		}

		if( xml.isStartElement() ) {
			if( xml.name() == "direction" ) {
				prediction.dirTitle = xml.attributes().value( "title" ).toString();
			} else if( xml.name() == "prediction" ) {
				QXmlStreamAttributes attributes = xml.attributes();
				prediction.seconds = attributes.value( "seconds" ).toString();
				prediction.minutes = attributes.value( "minutes" ).toString();
				prediction.dirTag = attributes.value( "dirTag" ).toString();
				prediction.vehicle = attributes.value( "vehicle" ).toString();
				
				bool dupe = false;
				for( int i = 0; !dupe && i < predictions.length(); i++ )
					if( predictions.at( i ).vehicle == prediction.vehicle && predictions.at( i ).seconds == prediction.seconds  )
						dupe = true;

				if( !dupe )
					predictions.append( prediction );
			} else if( xml.name() == "message" ) {
				if( !message->isEmpty() )
					message->append( "\n" );
				QString msg = xml.attributes().value( "text" ).toString();
				msg.replace( '\n', ' ' );
				message->append( msg );
			}
		}
	}

	qSort( predictions );

	return predictions;
}
