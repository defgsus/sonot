/***************************************************************************

Copyright (C) 2016  stefan.berke @ modular-audio-graphics.com

This source is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 3.0 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this software; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

****************************************************************************/

#ifndef SONOTSRC_SCOREVIEW_H
#define SONOTSRC_SCOREVIEW_H

#include <QWidget>

#include "core/Score.h"

namespace Sonot {

class Note;
class ScoreDocument;
class ScoreEditor;

class ScoreView : public QWidget
{
    Q_OBJECT
public:
    explicit ScoreView(QWidget *parent = 0);
    ~ScoreView();

    // ----- getter -----

    bool isAssigned() const { return scoreDocument() != nullptr; }

    ScoreDocument* scoreDocument() const;
    ScoreEditor* editor() const;

    QRect mapFromDocument(const QRectF& docSpace);

    // ----- setter -----

    /** Ownership is not taken */
    void setDocument(ScoreDocument*);

signals:

    void noteEntered(const Note& n);

public slots:

    /** Go to top-left of given page.
        Leaves scale or rotation unchanged */
    void goToPage(int pageIndex, double margin_mm = 5.);

    /** Sets the transformation such that the given rectangle
        in document-space is completely visible */
    void showRect(const QRectF& rect_mm, double margin_mm = 5.);
    /** Sets the transformation such that the given page
        is completely visible. */
    void showPage(int pageIndex, double margin_mm = 5.);

    /** Move the view such that p is top-left,
        leaves scaling in place */
    void moveToPoint(const QPointF& p);

    /** Ensures the point to be within view.
        Returns true, if the view has changed */
    bool ensureIndexVisible(const Score::Index& );

    void updateIndex(const Score::Index&, double margin = 1.);

    void setPlayingIndex(const Score::Index&);

protected:

    void keyPressEvent(QKeyEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void paintEvent(QPaintEvent*) override;

private:
    struct Private;
    Private* p_;
};

} // namespace Sonot

#endif // SONOTSRC_SCOREVIEW_H
