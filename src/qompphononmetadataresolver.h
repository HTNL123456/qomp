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

#ifndef QOMPPHONONMETADATARESOLVER_H
#define QOMPPHONONMETADATARESOLVER_H

#include "qompmetadataresolver.h"

#include <Phonon/MediaObject>

class QompPhononMetaDataResolver : public QompMetaDataResolver
{
	Q_OBJECT
public:
	QompPhononMetaDataResolver(QObject *parent = 0);
	~QompPhononMetaDataResolver();
	virtual void resolve(const TuneList& tunes);

private slots:
	void metaDataReady();
	void resolverStateChanged(Phonon::State newState, Phonon::State oldState);
	void totalTimeChanged(qint64 msec);

private:
	Phonon::MediaSource objectForTune(const Tune& tune) const;

private:
	Phonon::MediaObject* resolver_;
};

#endif // QOMPPHONONMETADATARESOLVER_H