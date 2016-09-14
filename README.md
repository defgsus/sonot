# Sonot - Spanish Organ Notation

This software is totally under development.

The main purpose is to enter 16th century spanish organ tablature and translate it into modern formats. First milestone is live editing and playback of tablature data and export into common music notation.

Below is the proposed vocabulary and class hierarchy.

Framework: Qt, License: GPL

## VOCABULARY

      Bar
     /  \     Note
    /    \    |
    |1 2 |5 6 7 8| row \                . .| ...|
    |2   |7''    | row  > line of score  . | .. |
    |  4 |p   3  | row /                   | .  |
    \                                          /
     \--------------NoteStream----------------/

          *top annotation
    |1   |p 6 7 8|
    |  3 |    8 9|
       *bot. annotation



    page
    |------------|
    | |NN|N |  | | row/staff/voice \
    | |N |  | N| | row/staff/voice / line
    |            |
    | |N |N | N| |
    | |NN|N |N | |
    |------------|
       || || ||
        column


## GENERAL DATA

### Note
- 1,, - 7'', p, space, ..., annotation, newline, newpage, ...

### Bar
- length (number Notes)
- data[length] of Note

### NoteStream
- voices
- data[voice] of Bar

### Score::Index
- stream
- bar
- row
- column

### Score
- data[] of NoteStream
- Score::Index

### ScoreEditor
- undo/redo
- ScoreEditorCursor
- add/del/change
    - setScore(Score)
    - insertNote(Index, n)
    - insertBars(Index, QList<Bar> rows)
    - changeNote(Index, n)
    - changeBar(Index, b)
    - deleteNote(Index)
    - deleteBar(Index)
    - deleteStream(Index)
- Qt signals
    - scoreChanged()
    - noteValuesChanged(QList<Index>)
    - barsChanged(QList<Index>)
    - streamChanged(Index)
    - notesAboutToBeDeleted(QList<Index>)
    - notesDeleted(QList<Index>)
    - barsAboutToBeDeleted(QList<Index>)
    - barsDeleted(QList<Index>)
    - streamsAboutToBeDeleted(QList<Index>)

### ScoreEditorCursor
- move within bar
- move within NoteStream
- set/insert/delete note
- insert/delete bar
- insert/delete row


## GUI/DRAWING

### TextItem
- alignment
- font
- bounding-box
- margin

### PageAnnotation
- TextItem[] (title, header, footer, pagenum, ...)

### PageAnnotationTemplate
- PageAnnotation[] (1st, odd, even, last, ...)

### PageSize
- width/height (mm)
- preset

### PageLayout
- PageSize
- page margins[odd/even]
- portrait/landscape
- PageAnnotationTemplate

### ScoreLayout
- row spacing
- line spacing
- note spacing
- min/max Bar width

### ScoreItem
- bar, note, TextItem, ...
- bounding box
- draw()

### ScoreDocument
- Score
- ScoreEditor
- PageLayout (global)
- ScoreLayout (global)
- optional override, per-page or until-next:
    - PageMargins
    - PageAnnotation
    - ScoreLayout
- createScoreItems(Score, pageIndex)
- bool pageChanged[]

### ScoreView
- ScoreDocument
- renderPage(QPainter*, pageIndex)
