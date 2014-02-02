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

#ifndef QOMP_H
#define QOMP_H

#include <QObject>
#include <QModelIndex>

#include "common.h"

class QompMainWin;
class QompPlayListModel;
class QompPlayer;
class Tune;


class QompCon : public QObject
{
	Q_OBJECT
public:
	QompCon(QObject *parent = 0);
	~QompCon();

	void init();

private slots:
	void exit();
	void updateOptions();

	void actPlayNext();
	void actPlayPrev();
	void actPlay();
	void actPause();
	void actStop();

	void actMediaActivated(const QModelIndex& index);
	void actMediaClicked(const QModelIndex& index);

	void actMuteToggle(bool mute);
	void actSeek(int ms);
	void actSetVolume(qreal vol);
	void actSavePlaylist();
	void actLoadPlaylist();
	void setTunes(const QList<Tune*>& tunes);

	void actClearPlaylist();
	void actRemoveSelected(const QModelIndexList& list);

	void actDoSettings();
	void actCheckForUpdates();
	void actAboutQomp();

	void actDownloadTune(Tune* tune);
	void actToggleTuneState(Tune* tune);
	void actRemoveTune(Tune* tune);

	void playNext();

//protected:
//	bool eventFilter(QObject *obj, QEvent *e);

private:
	void setupMainWin();
	void setupPlayer();
	void setupModel();

	void stopPlayer();

private:
	QompMainWin* mainWin_;
	QompPlayListModel* model_;
	QompPlayer* player_;
};

#endif // QOMP_H