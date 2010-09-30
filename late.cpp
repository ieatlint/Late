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

#include "late.h"
#include <QDebug>

bool operator == ( const StopInfo &a, const StopInfo &b ) {
	if( a.agency == b.agency && a.route == b.route && a.direction == b.direction && a.stop == b.stop )
		return true;
	return false;
}

QDataStream & operator << ( QDataStream &s, const StopInfo &t ) {
	s << t.agency;
	s << t.route;
	s << t.direction;
	s << t.stop;
	return s;
}

QDataStream & operator >> ( QDataStream &s, StopInfo &t ) {
	s >> t.agency;
	s >> t.route;
	s >> t.direction;
	s >> t.stop;
	return s;
}

Late::Late() {// : QMainWindow(parent) {
	#ifdef Q_WS_MAEMO_5
	setAttribute( Qt::WA_Maemo5PortraitOrientation, true );
	#else
	resize( 480, 744 );
	#endif

	connect( &nbus, SIGNAL( notice( int, QString ) ), this, SLOT( notice( int, QString ) ) );

	itemSelectLock = false;

	nbusInfo.agency.tag = settings.value( "last/agency/tag" ).toString();
	nbusInfo.agency.title = settings.value( "last/agency/title" ).toString();
	if( !nbusInfo.agency.tag.isEmpty() )
		nbus.setAgency( nbusInfo.agency.tag, nbusInfo.agency.title );

	nbusInfo.route.tag = settings.value( "last/route/tag" ).toString();
	nbusInfo.route.title = settings.value( "last/route/title" ).toString();
	if( !nbusInfo.route.tag.isEmpty() )
		nbus.setRoute( nbusInfo.route.tag, nbusInfo.route.title );

	nbusInfo.direction.tag = settings.value( "last/direction/tag" ).toString();
	nbusInfo.direction.title = settings.value( "last/direction/title" ).toString();
	if( !nbusInfo.direction.tag.isEmpty() )
		nbus.setDirection( nbusInfo.direction.tag, nbusInfo.direction.title );

	nbusInfo.stop.tag = settings.value( "last/stop/tag" ).toString();
	nbusInfo.stop.title = settings.value( "last/stop/title" ).toString();
	
	loadBookmarks();
	loadHistory();

	mainWindowInit();
}

Late::~Late() {
	settings.setValue( "last/agency/tag", nbusInfo.agency.tag );
	settings.setValue( "last/agency/title", nbusInfo.agency.title );
	settings.setValue( "last/route/tag", nbusInfo.route.tag );
	settings.setValue( "last/route/title", nbusInfo.route.title );
	settings.setValue( "last/direction/tag", nbusInfo.direction.tag );
	settings.setValue( "last/direction/title", nbusInfo.direction.title );
	settings.setValue( "last/stop/tag", nbusInfo.stop.tag );
	settings.setValue( "last/stop/title", nbusInfo.stop.title );

	saveBookmarks();
	saveHistory();
}

void Late::notice( int type, QString message ) {
	if( type == INFO ) {
	#ifdef Q_WS_MAEMO_5
		QMaemo5InformationBox infoBox;
		infoBox.information( 0, message, 2000 );
		infoBox.show();
	#else
		QMessageBox mbox( this );
		mbox.setText( message );
		mbox.exec();
	#endif
	} else if( type == CRITICAL ) {
		QMessageBox mbox( this );
		mbox.setText( message );
		mbox.exec();
	} else {
		qDebug() << "notice" << type << message;
	}
}


void Late::mainWindowInit() {
	mainWidget = new QWidget;
	mainLayout = new QVBoxLayout( mainWidget );

	mainLayout->setContentsMargins( 20, 0, 20, 0 );

	mainList = new QListWidget(  );
	//connect( mainList, SIGNAL( itemActivated( QListWidgetItem * ) ), this, SLOT( mainSelected( QListWidgetItem * ) ) ); 
	connect( mainList, SIGNAL( itemClicked( QListWidgetItem * ) ), this, SLOT( mainSelected( QListWidgetItem * ) ) ); 

	QListWidgetItem *item;
	QString text;

	item = new QListWidgetItem;
	if( nbusInfo.agency.tag.isEmpty() )
		text = "  Select Transit Agency";
	else
		text = "  Agency: " + nbusInfo.agency.title;

	item->setText( text );
	item->setStatusTip( "agency" );

	mainList->addItem( item );

	if( !nbusInfo.agency.tag.isEmpty() ) {
		item = new QListWidgetItem;
		if( nbusInfo.route.tag.isEmpty() )
			text = "  Select Transit Line";
		else
			text = "  Line: " + nbusInfo.route.title;

		item->setText( text );
		item->setStatusTip( "route" );
		mainList->addItem( item );
	}


	if( !nbusInfo.route.tag.isEmpty() ) {
		item = new QListWidgetItem;
		if( nbusInfo.direction.tag.isEmpty() )
			text = "  Select Direction/Destination";
		else
			text = "  Direction: " + nbusInfo.direction.title;
		
		item->setText( text );
		item->setStatusTip( "direction" );
		mainList->addItem( item );
	}

	if( !nbusInfo.direction.tag.isEmpty() ) {
		item = new QListWidgetItem;
		if( nbusInfo.stop.tag.isEmpty() )
			text = "  Select Stop";
		else
			text = "  Stop: " + nbusInfo.stop.title;

		item->setText( text );
		item->setStatusTip( "stop" );
		mainList->addItem( item );
	}

	mainLayout->addWidget( mainList );
	mainLayout->setStretch( 0, 1 );

	mainLayout->addStretch();

	/*
	mainBtnSettings = new QPushButton( "Settings" );
	mainLayout->addWidget( mainBtnSettings );
	*/
	QPushButton *button = new QPushButton( "History" );
	connect( button, SIGNAL( clicked() ), this, SLOT( historyWindowInit() ) );
	mainLayout->addWidget( button );

	button = new QPushButton( "Bookmarks" );
	connect( button, SIGNAL( clicked() ), this, SLOT( bookmarkWindowInit() ) );
	mainLayout->addWidget( button );

	button = new QPushButton( "Check Arrivals" );
	if( nbusInfo.stop.tag.isEmpty() )
		button->setEnabled( false );
	else
		connect( button, SIGNAL( clicked() ), this, SLOT( predWindowInit() ) );
	mainLayout->addWidget( button );

	mainWidget->show();
	setCentralWidget( mainWidget );

}

void Late::mainSelected( QListWidgetItem *item ) {
	itemSelect( item->statusTip() );
}

void Late::itemSelect( QString type ) {
	if( itemSelectLock )
		return;
	itemSelectLock = true;
	QWidget *widget = new QWidget;
	QVBoxLayout *layout = new QVBoxLayout;
	QListWidget *list = new QListWidget;
	QListWidgetItem *item;
	QList<TitleTuple> nbusList;
	TitleTuple nbusItem;

	#ifdef Q_WS_MAEMO_5
	setAttribute( Qt::WA_Maemo5ShowProgressIndicator, true );
	#endif

	setCentralWidget( new QLabel( "<div align=center>Loading ...</div>" ) );

	QPushButton *backBtn = new QPushButton( "Back" );
	connect( backBtn, SIGNAL( clicked() ), this, SLOT( mainWindowInit() ) );
	layout->addWidget( backBtn );


	if( type == "agency" ) {
		nbusList = nbus.agencyList();
		connect( list, SIGNAL( itemActivated( QListWidgetItem * ) ), this, SLOT( agencySelected( QListWidgetItem * ) ) );
	} else if( type == "route" ) {
		nbusList = nbus.routeList();
		connect( list, SIGNAL( itemActivated( QListWidgetItem * ) ), this, SLOT( routeSelected( QListWidgetItem * ) ) );
	} else if( type == "direction" ) {
		nbusList = nbus.directionList();
		connect( list, SIGNAL( itemActivated( QListWidgetItem * ) ), this, SLOT( directionSelected( QListWidgetItem * ) ) );
	} else if( type == "stop" ) {
		QList<StopData> stops;
		StopData stop;

		stops = nbus.stopList();
		connect( list, SIGNAL( itemActivated( QListWidgetItem * ) ), this, SLOT( stopSelected( QListWidgetItem * ) ) );

		for( int i = 0; i < stops.length(); i++ ) {
			stop = stops.at( i );
			item = new QListWidgetItem( stop.title, list );
			item->setStatusTip( stop.tag );
		}

	} else {
		qWarning() << "Unknown type" << type;
	}

	if( type != "stop" ) {
		for( int i = 0; i < nbusList.length(); i ++ ) {
			nbusItem = nbusList.at( i );
			item = new QListWidgetItem( nbusItem.title, list );
			item->setStatusTip( nbusItem.tag );
		}
	}

	if( list->count() > 0 ) {
		layout->addWidget( list );
		widget->setLayout( layout );
		setCentralWidget( widget );
	} else {
		qWarning() << "Failed to load list for" << type;
	}

	#ifdef Q_WS_MAEMO_5
	setAttribute( Qt::WA_Maemo5ShowProgressIndicator, false );
	#endif

	itemSelectLock = false;
}

void Late::agencySelected( QListWidgetItem *item ) {
	nbusInfo.agency.tag = item->statusTip();
	nbusInfo.agency.title = item->text();

	nbusInfo.route.tag.clear();
	nbusInfo.direction.tag.clear();
	nbusInfo.stop.tag.clear();

	nbus.setAgency( nbusInfo.agency.tag, nbusInfo.agency.title );

	itemSelect( "route" );
}

void Late::routeSelected( QListWidgetItem *item ) {
	nbusInfo.route.tag = item->statusTip();
	nbusInfo.route.title = item->text();

	nbusInfo.direction.tag.clear();
	nbusInfo.stop.tag.clear();

	nbus.setRoute( nbusInfo.route.tag, nbusInfo.route.title );

	itemSelect( "direction" );
}

void Late::directionSelected( QListWidgetItem *item ) {
	nbusInfo.direction.tag = item->statusTip();
	nbusInfo.direction.title = item->text();

	nbusInfo.stop.tag.clear();

	nbus.setDirection( nbusInfo.direction.tag, nbusInfo.direction.title );

	itemSelect( "stop" );
}

void Late::stopSelected( QListWidgetItem *item ) {
	nbusInfo.stop.tag = item->statusTip();
	nbusInfo.stop.title = item->text();

	if( nbusInfo.stop.tag.isEmpty() )
		mainWindowInit();
	else
		predWindowInit();
}

/* Prediction Screen */

void Late::predWindowInit() {
	QWidget *predWidget = new QWidget;
	QVBoxLayout *predLayout = new QVBoxLayout( predWidget );

	#ifdef Q_WS_MAEMO_5
	setAttribute( Qt::WA_Maemo5ShowProgressIndicator, true );
	#endif

	setCentralWidget( new QLabel( "<div align=center>Loading ...</div>" ) );

	QPushButton *backBtn = new QPushButton( "Back" );
	connect( backBtn, SIGNAL( clicked() ), this, SLOT( mainWindowInit() ) );
	predLayout->addWidget( backBtn );

	QFormLayout *summary = new QFormLayout;
	
	summary->addRow( "Agency:", new QLabel( nbusInfo.agency.title ) );
	summary->addRow( "Line:", new QLabel( nbusInfo.route.title ) );
	summary->addRow( "Direction:", new QLabel( nbusInfo.direction.title ) );
	summary->addRow( "Stop:", new QLabel( nbusInfo.stop.title ) );

	predLayout->addLayout( summary );

	QString message;
	QList<PredData> predictions = nbus.getPredictionsSimple( &message, nbusInfo.stop.tag );
	PredData prediction;

	QString arrivals = "<div align=center>";
	if( predictions.length() < 1 ) {
		arrivals.append( "<span style=\"font-size:32pt;\">No Estimate</span>" );
	}

	for( int i = 0; i < predictions.length(); i++ ) {
		prediction = predictions.at( i );

		arrivals.append( "<span style=\"font-size:32pt;\">" );
		if( prediction.minutes == "0" )
			arrivals.append( "Arriving" );
		else
			arrivals.append( prediction.minutes + " minutes" );
		arrivals.append( "</span><br>\n" );

		arrivals.append( "(" + prediction.dirTitle + ")<br>\n" );
	}

	arrivals.append( "</div>" );


	QScrollArea *sa = new QScrollArea;
	QWidget *widget = new QWidget;
	QVBoxLayout *lay = new QVBoxLayout( widget );
	sa->setWidgetResizable( true );

	widget->setLayout( lay );
	sa->setWidget( widget );

	lay->addWidget( new QLabel( arrivals ) );

	if( !message.isEmpty() ) {
		QLabel *msg = new QLabel( message );
		msg->setWordWrap( true );
		lay->addWidget( msg );
	}

	QPushButton *bookmark = new QPushButton( "Add Bookmark" );
	connect( bookmark, SIGNAL( clicked() ), this, SLOT( addBookmark() ) );
	lay->addWidget( bookmark );

	widget->show();
	predLayout->addWidget( sa );

	QPushButton *refreshBtn = new QPushButton( "Refresh" );
	connect( refreshBtn, SIGNAL( clicked() ), this, SLOT( predWindowInit() ) );
	predLayout->addWidget( refreshBtn );

	predWidget->setLayout( predLayout );
	setCentralWidget( predWidget );

	#ifdef Q_WS_MAEMO_5
	setAttribute( Qt::WA_Maemo5ShowProgressIndicator, false );
	#endif

	addHistory();
}

void Late::addHistory() {
	history.prepend( nbusInfo );

	// only allow a stop to be in the list once
	for( int i = 1; history.length() > i; i++ ) {
		if( history.at( i ) == nbusInfo ) {
			history.removeAt( i );
			break;
		}
	}

	//trim the list to be under 10 .. make this a user-configurable variable sometime
	while( history.length() > 10 ) {
		history.removeLast();
	}
}

void Late::addBookmark() {
	for( int i = 0; bookmarks.length() > i; i++ ) {
		if( bookmarks.at( i ) == nbusInfo ) {
			return;
		}
	}

	bookmarks.append( nbusInfo );
}


void Late::bookmarkWindowInit() {
	QWidget *widget = new QWidget;
	QVBoxLayout *vbox = new QVBoxLayout( widget );

	QPushButton *button = new QPushButton( "Back" );
	connect( button, SIGNAL( clicked() ), this, SLOT( mainWindowInit() ) );
	vbox->addWidget( button );


	QListWidget *list = new QListWidget;
	QListWidgetItem *item;

	StopInfo curInfo;
	
	for( int i = 0; i < bookmarks.length(); i++ ) {
		curInfo = bookmarks.at( i );

		QString title = QString( "%1\n%2 - %3\n%4" ).arg( curInfo.agency.title ).arg( curInfo.route.tag ).arg( curInfo.direction.title ).arg( curInfo.stop.title );

		item = new QListWidgetItem( title, list );
	}
	
	if( bookmarks.isEmpty() ) {
		// Set this to not be clickable
		new QListWidgetItem( "Bookmark list is empty", list );
	} else {
		connect( list, SIGNAL( itemActivated( QListWidgetItem * ) ), this, SLOT( loadBookmarkInfo( QListWidgetItem * ) ) );
	}

	vbox->addWidget( list );

	widget->show();
	setCentralWidget( widget );
}

void Late::loadBookmarkInfo( QListWidgetItem *item ) {
	int i = item->listWidget()->currentRow();

	nbusInfo = bookmarks.at( i );

	nbus.setAgency( nbusInfo.agency.tag, nbusInfo.agency.title );
	nbus.setRoute( nbusInfo.route.tag, nbusInfo.route.title );
	nbus.setDirection( nbusInfo.direction.tag, nbusInfo.direction.title );

	predWindowInit();
}

void Late::saveBookmarks() {
	QDir dir;
	QString fileName;

	fileName = QDesktopServices::storageLocation( QDesktopServices::DataLocation );
	if( !dir.exists( fileName ) ) {//this should be made by NextBus module
		notice( CRITICAL, "Failed to save bookmarks: data dir doesn't exist" );
		return;
	}

	fileName.append( "/bookmarks" );
	QFile bmFile;
	bmFile.setFileName( fileName );
	if( !bmFile.open( QIODevice::WriteOnly ) ) {
		notice( CRITICAL, "Failed to save bookmarks: unable to write file" );
		return;
	} 

	QDataStream out( &bmFile );
	out << bookmarks;

	bmFile.close();
}

void Late::loadBookmarks() {
	QFile bmFile;
	QString fileName;

	fileName = QDesktopServices::storageLocation( QDesktopServices::DataLocation ) + "/bookmarks";

	bmFile.setFileName( fileName );
	if( bmFile.exists() ) {
		if( !bmFile.open( QIODevice::ReadOnly ) ) {
			notice( WARNING, "Unable to open bookmarks file for reading\nBookmarks will be unavailable" );
			return;
		}

		QDataStream in( &bmFile );
		in >> bookmarks;

		bmFile.close();
	}
}

void Late::historyWindowInit() {
	QWidget *widget = new QWidget;
	QVBoxLayout *vbox = new QVBoxLayout( widget );

	QPushButton *button = new QPushButton( "Back" );
	connect( button, SIGNAL( clicked() ), this, SLOT( mainWindowInit() ) );
	vbox->addWidget( button );

	QListWidget *list = new QListWidget;
	QListWidgetItem *item;

	StopInfo curInfo;
	
	for( int i = 0; i < history.length(); i++ ) {
		curInfo = history.at( i );

		QString title = QString( "%1\n%2 - %3\n%4" ).arg( curInfo.agency.title ).arg( curInfo.route.tag ).arg( curInfo.direction.title ).arg( curInfo.stop.title );

		item = new QListWidgetItem( title, list );
	}
	
	if( history.isEmpty() ) {
		// Set this to not be clickable?
		new QListWidgetItem( "History list is empty", list );
	} else {
		connect( list, SIGNAL( itemActivated( QListWidgetItem * ) ), this, SLOT( loadHistoryInfo( QListWidgetItem * ) ) );
	}

	vbox->addWidget( list );

	widget->show();
	setCentralWidget( widget );
}

void Late::loadHistoryInfo( QListWidgetItem *item ) {
	int i = item->listWidget()->currentRow();

	nbusInfo = history.at( i );

	nbus.setAgency( nbusInfo.agency.tag, nbusInfo.agency.title );
	nbus.setRoute( nbusInfo.route.tag, nbusInfo.route.title );
	nbus.setDirection( nbusInfo.direction.tag, nbusInfo.direction.title );

	predWindowInit();
}

void Late::saveHistory() {
	QDir dir;
	QString fileName;

	fileName = QDesktopServices::storageLocation( QDesktopServices::DataLocation );
	if( !dir.exists( fileName ) ) {//this should be made by NextBus module
		notice( CRITICAL, "Failed to save history: data dir doesn't exist" );
		return;
	}

	fileName.append( "/history" );
	QFile hFile;
	hFile.setFileName( fileName );
	if( !hFile.open( QIODevice::WriteOnly ) ) {
		notice( CRITICAL, "Failed to save history: unable to write file" );
		return;
	} 

	QDataStream out( &hFile );
	out << history;

	hFile.close();
}

void Late::loadHistory() {
	QFile hFile;
	QString fileName;

	fileName = QDesktopServices::storageLocation( QDesktopServices::DataLocation ) + "/history";

	hFile.setFileName( fileName );
	if( hFile.exists() ) {
		if( !hFile.open( QIODevice::ReadOnly ) ) {
			notice( WARNING, "Unable to open history file for reading\nHistory will be unavailable" );
			return;
		}

		QDataStream in( &hFile );
		in >> history;

		hFile.close();
	}
}
