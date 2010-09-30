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

#ifndef LATE_H
#define LATE_H

#include <QtGui/QMainWindow>
#include <QVBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QFormLayout>
#include <QLabel>
#include <QErrorMessage>
#include <QMessageBox>
#include <QScrollArea>
#include <QSettings>

#ifdef Q_WS_MAEMO_5
#include <QtMaemo5>
#endif

#define INFO 1
#define CRITICAL 2
#define WARNING 3

#include "nextbus.h"


class StopInfo {
	public:
		TitleTuple agency;
		TitleTuple route;
		TitleTuple direction;
		StopData stop;
};

class Late : public QMainWindow {
	Q_OBJECT

	public:
		Late();
		~Late();

	private:
		NextBus nbus;
		bool itemSelectLock;
		QSettings settings;

		StopInfo nbusInfo;
		QList<StopInfo> history;
		QList<StopInfo> bookmarks;

		/* Main Window */
		QWidget *mainWidget;
		QVBoxLayout *mainLayout;

		QListWidget *mainList;

		void itemSelect( QString type );

		void agencySelect();
		QListWidgetItem *mainItemAgency;
		
		void routeSelect();
		QListWidgetItem *mainItemRoute;
		QListWidgetItem *mainItemDirection;
		QListWidgetItem *mainItemStop;

		QPushButton *mainBtnSettings;

		void loadBookmarks();
		void saveBookmarks();
		void loadHistory();
		void saveHistory();
	
	private slots:
		void notice( int type, QString message );
		void mainWindowInit();
		void mainSelected( QListWidgetItem *item );
		void agencySelected( QListWidgetItem *item );
		void routeSelected( QListWidgetItem *item );
		void directionSelected( QListWidgetItem *item );
		void stopSelected( QListWidgetItem *item );

		void predWindowInit();
		void historyWindowInit();
		void bookmarkWindowInit();

		void addBookmark();
		void loadBookmarkInfo( QListWidgetItem * );
		void addHistory();
		void loadHistoryInfo( QListWidgetItem * );

};

QDataStream & operator << ( QDataStream &s, const StopInfo &t );
QDataStream & operator >> ( QDataStream &s, const StopInfo &t );
#endif // LATE_H
