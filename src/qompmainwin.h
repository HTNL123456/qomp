/*
 * Copyright (C) 2013  Khryukin Evgeny
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef QOMPMAINWIN_H
#define QOMPMAINWIN_H

#include <QMainWindow>

namespace Ui {
	class QompMainWin;
}

class QompPlayer;
class PlayListModel;
class QModelIndex;
class QNetworkAccessManager;

class QompMainWin : public QMainWindow
{
	Q_OBJECT
	
public:
	QompMainWin(QWidget *parent = 0);
	~QompMainWin();

private slots:
	void actPlayActivated();
	void actPrevActivated();
	void actNextActivated();
	void actStopActivated();
	void actOpenActivated();
	void actClearActivated();
	void mediaActivated(const QModelIndex& index);
	void mediaClicked(const QModelIndex& index);
	void updatePlayIcon();
	void setCurrentPosition(qint64 ms);
	void playNext();
	void doOptions();

	void loadPlaylist();
	void savePlaylist();

	void trayDoubleclicked();
	void trayActivated(Qt::MouseButton);

signals:
	void exit();

protected:
	void closeEvent(QCloseEvent *e);

private:
	void savePlaylist(const QString& fileName);
	void updateTuneInfoFrame();
	
private:
	Ui::QompMainWin *ui;
	QompPlayer* player_;
	PlayListModel* model_;
	QNetworkAccessManager* nam_;
};

#endif // QOMPMAINWIN_H