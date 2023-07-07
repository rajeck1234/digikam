/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 1997-04-21
 * Description : Date selection table.
 *
 * SPDX-FileCopyrightText: 2011-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 1997      by Tim D. Gilman <tdgilman at best dot org>
 * SPDX-FileCopyrightText: 1998-2001 by Mirko Boehm <mirko at kde dot org>
 * SPDX-FileCopyrightText: 2007      by John Layt <john at layt dot net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_DDATE_TABLE_H
#define DIGIKAM_DDATE_TABLE_H

// Qt includes

#include <QLineEdit>
#include <QDateTime>

class QMenu;

namespace Digikam
{

/**
 * This is a support class for the DDatePicker class. It just
 * draws the calendar table without titles, but could theoretically
 * be used as a standalone.
 *
 * When a date is selected by the user, it emits a signal:
 * dateSelected(QDate)
 *
 */
class DDateTable : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QDate date READ date WRITE setDate)
    Q_PROPERTY(bool popupMenu READ popupMenuEnabled WRITE setPopupMenuEnabled)

public:

    enum BackgroundMode
    {
        NoBgMode = 0,
        RectangleMode,
        CircleMode
    };

public:

    explicit DDateTable(QWidget* const parent = nullptr);
    explicit DDateTable(const QDate& dt, QWidget* const parent = nullptr);
    ~DDateTable() override;

    /**
     * Returns a recommended size for the widget.
     * To save some time, the size of the largest used cell content is
     * calculated in each paintCell() call, since all calculations have
     * to be done there anyway. The size is stored in maxCell. The
     * sizeHint() simply returns a multiple of maxCell.
     */
    QSize sizeHint() const override;

    /**
     * Set the font size of the date table.
     */
    void setFontSize(int size);

    /**
     * Select and display this date.
     */
    bool setDate(const QDate& date);

    /**
     * @returns the selected date.
     */
    const QDate& date() const;

    /**
     * Enables a popup menu when right clicking on a date.
     *
     * When it's enabled, this object emits a aboutToShowContextMenu signal
     * where you can fill in the menu items.
     */
    void setPopupMenuEnabled(bool enable);

    /**
     * Returns if the popup menu is enabled or not
     */
    bool popupMenuEnabled() const;

    /**
     * Makes a given date be painted with a given foregroundColor, and background
     * (a rectangle, or a circle/ellipse) in a given color.
     */
    void setCustomDatePainting(const QDate& date, const QColor& fgColor,
                               BackgroundMode bgMode = NoBgMode,
                               const QColor& bgColor = QColor());

    /**
     * Unsets the custom painting of a date so that the date is painted as usual.
     */
    void unsetCustomDatePainting(const QDate& dt);

protected:

    /**
     * calculate the position of the cell in the matrix for the given date.
     * The result is the 0-based index.
     */
    virtual int posFromDate(const QDate& dt);

    /**
     * calculate the date that is displayed at a given cell in the matrix. pos is the
     * 0-based index in the matrix. Inverse function to posForDate().
     */
    virtual QDate dateFromPos(int pos);

    void paintEvent(QPaintEvent* e) override;

    /**
     * React on mouse clicks that select a date.
     */
    void mousePressEvent(QMouseEvent* e) override;
    void wheelEvent(QWheelEvent* e) override;
    void keyPressEvent(QKeyEvent* e) override;
    void focusInEvent(QFocusEvent* e) override;
    void focusOutEvent(QFocusEvent* e) override;

    /**
     * Cell highlight on mouse hovering
     */
    bool event(QEvent* e) override;

Q_SIGNALS:

    /**
     * The selected date changed.
     */
    void dateChanged(const QDate& date);

    /**
     * This function behaves essentially like the one above.
     * The selected date changed.
     * @param cur The current date
     * @param old The date before the date was changed
     */
    void dateChanged(const QDate& cur, const QDate& old);

    /**
     * A date has been selected by clicking on the table.
     */
    void tableClicked();

    /**
     * A popup menu for a given date is about to be shown (as when the user
     * right clicks on that date and the popup menu is enabled). Connect
     * the slot where you fill the menu to this signal.
     */
    void aboutToShowContextMenu(QMenu* menu, const QDate& dt);

private:

    void initWidget(const QDate& dt);
    void initAccels();
    void paintCell(QPainter* painter, int row, int col);

private:

    class Private;
    Private* const d;

    friend class Private;

    Q_DISABLE_COPY(DDateTable)
};

} // namespace Digikam

#endif // DIGIKAM_DDATE_TABLE_H
