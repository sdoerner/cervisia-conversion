/**********************************************************************
** $Id$
**
** Definition of QtTableView class
**
** Created : 941115
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file contains a class moved out of the TQt GUI Toolkit API. It
** may be used, distributed and modified without limitation.
**
**********************************************************************/

#ifndef TQTTABLEVIEW_H
#define TQTTABLEVIEW_H

#ifndef TQT_H
#include "tqframe.h"
#endif // TQT_H

#ifndef TQT_NO_TQTTABLEVIEW

class TQScrollBar;
class TQCornerSquare;


class QtTableView : public TQFrame
{
    Q_OBJECT
  TQ_OBJECT
public:
    virtual void setBackgroundColor( const TQColor & );
    virtual void setPalette( const TQPalette & );
    void	show();

    void	tqrepaint( bool erase=TRUE );
    void	tqrepaint( int x, int y, int w, int h, bool erase=TRUE );
    void	tqrepaint( const TQRect &, bool erase=TRUE );

protected:
    QtTableView( TQWidget *tqparent=0, const char *name=0, WFlags f=0 );
   ~QtTableView();

    int		numRows()	const;
    virtual void setNumRows( int );
    int		numCols()	const;
    virtual void setNumCols( int );

    int		topCell()	const;
    virtual void setTopCell( int row );
    int		leftCell()	const;
    virtual void setLeftCell( int col );
    virtual void setTopLeftCell( int row, int col );

    int		xOffset()	const;
    virtual void setXOffset( int );
    int		yOffset()	const;
    virtual void setYOffset( int );
    virtual void setOffset( int x, int y, bool updateScrBars = TRUE );

    virtual int cellWidth( int col );
    virtual int cellHeight( int row );
    int		cellWidth()	const;
    int		cellHeight()	const;
    virtual void setCellWidth( int );
    virtual void setCellHeight( int );

    virtual int totalWidth();
    virtual int totalHeight();

    uint	tableFlags()	const;
    bool	testTableFlags( uint f ) const;
    virtual void setTableFlags( uint f );
    void	clearTableFlags( uint f = ~0 );

    bool	autoUpdate()	 const;
    virtual void setAutoUpdate( bool );

    void	updateCell( int row, int column, bool erase=TRUE );

    TQRect	cellUpdateRect() const;
    TQRect	viewRect()	 const;

    int		lastRowVisible() const;
    int		lastColVisible() const;

    bool	rowIsVisible( int row ) const;
    bool	colIsVisible( int col ) const;

    TQScrollBar *verticalScrollBar() const;
    TQScrollBar *horizontalScrollBar() const;

private slots:
    void	horSbValue( int );
    void	horSbSliding( int );
    void	horSbSlidingDone();
    void	verSbValue( int );
    void	verSbSliding( int );
    void	verSbSlidingDone();

protected:
    virtual void paintCell( TQPainter *, int row, int col ) = 0;
    virtual void setupPainter( TQPainter * );

    void	paintEvent( TQPaintEvent * );
    void	resizeEvent( TQResizeEvent * );
    virtual void wheelEvent( TQWheelEvent *e );

    int		findRow( int yPos ) const;
    int		findCol( int xPos ) const;

    bool	rowYPos( int row, int *yPos ) const;
    bool	colXPos( int col, int *xPos ) const;

    int		maxXOffset();
    int		maxYOffset();
    int		maxColOffset();
    int		maxRowOffset();

    int		minViewX()	const;
    int		minViewY()	const;
    int		maxViewX()	const;
    int		maxViewY()	const;
    int		viewWidth()	const;
    int		viewHeight()	const;

    void	scroll( int xPixels, int yPixels );
    void	updateScrollBars();
    void	updateTableSize();

private:
    void	coverCornerSquare( bool );
    void	snapToGrid( bool horizontal, bool vertical );
    virtual void	setHorScrollBar( bool on, bool update = TRUE );
    virtual void	setVerScrollBar( bool on, bool update = TRUE );
    void	updateView();
    int		findRawRow( int yPos, int *cellMaxY, int *cellMinY = 0,
			    bool goOutsideView = FALSE ) const;
    int		findRawCol( int xPos, int *cellMaxX, int *cellMinX = 0,
			    bool goOutsideView = FALSE ) const;
    int		maxColsVisible() const;

    void	updateScrollBars( uint );
    void	updateFrameSize();

    void	doAutoScrollBars();
    void	showOrHideScrollBars();

    int		nRows;
    int		nCols;
    int		xOffs, yOffs;
    int		xCellOffs, yCellOffs;
    short	xCellDelta, yCellDelta;
    short	cellH, cellW;

    uint	eraseInPaint		: 1;
    uint	verSliding		: 1;
    uint	verSnappingOff		: 1;
    uint	horSliding		: 1;
    uint	horSnappingOff		: 1;
    uint	coveringCornerSquare	: 1;
    uint	sbDirty			: 8;
    uint	inSbUpdate		: 1;

    uint	tFlags;
    TQRect	cellUpdateR;

    TQScrollBar *vScrollBar;
    TQScrollBar *hScrollBar;
    TQCornerSquare *cornerSquare;

private:	// Disabled copy constructor and operator=
#if defined(TQ_DISABLE_COPY)
    QtTableView( const QtTableView & );
    QtTableView &operator=( const QtTableView & );
#endif
};


const uint Tbl_vScrollBar	= 0x00000001;
const uint Tbl_hScrollBar	= 0x00000002;
const uint Tbl_autoVScrollBar	= 0x00000004;
const uint Tbl_autoHScrollBar	= 0x00000008;
const uint Tbl_autoScrollBars	= 0x0000000C;

const uint Tbl_clipCellPainting = 0x00000100;
const uint Tbl_cutCellsV	= 0x00000200;
const uint Tbl_cutCellsH	= 0x00000400;
const uint Tbl_cutCells		= 0x00000600;

const uint Tbl_scrollLastHCell	= 0x00000800;
const uint Tbl_scrollLastVCell	= 0x00001000;
const uint Tbl_scrollLastCell	= 0x00001800;

const uint Tbl_smoothHScrolling = 0x00002000;
const uint Tbl_smoothVScrolling = 0x00004000;
const uint Tbl_smoothScrolling	= 0x00006000;

const uint Tbl_snapToHGrid	= 0x00008000;
const uint Tbl_snapToVGrid	= 0x00010000;
const uint Tbl_snapToGrid	= 0x00018000;


inline int QtTableView::numRows() const
{ return nRows; }

inline int QtTableView::numCols() const
{ return nCols; }

inline int QtTableView::topCell() const
{ return yCellOffs; }

inline int QtTableView::leftCell() const
{ return xCellOffs; }

inline int QtTableView::xOffset() const
{ return xOffs; }

inline int QtTableView::yOffset() const
{ return yOffs; }

inline int QtTableView::cellHeight() const
{ return cellH; }

inline int QtTableView::cellWidth() const
{ return cellW; }

inline uint QtTableView::tableFlags() const
{ return tFlags; }

inline bool QtTableView::testTableFlags( uint f ) const
{ return (tFlags & f) != 0; }

inline TQRect QtTableView::cellUpdateRect() const
{ return cellUpdateR; }

inline bool QtTableView::autoUpdate() const
{ return isUpdatesEnabled(); }

inline void QtTableView::tqrepaint( bool erase )
{ tqrepaint( 0, 0, width(), height(), erase ); }

inline void QtTableView::tqrepaint( const TQRect &r, bool erase )
{ tqrepaint( r.x(), r.y(), r.width(), r.height(), erase ); }

inline void QtTableView::updateScrollBars()
{ updateScrollBars( 0 ); }


#endif // TQT_NO_TQTTABLEVIEW

#endif // TQTTABLEVIEW_H
