# Spanish Organ Notation Software

## VOCABULARY

    messure
     /  \
    /    \
    |1 2 |5 6 7 8| line \
    |2   |7''    | line  > staff
    |  4 |p   3  | line /
    \            /
     \          /
      \--staff-/

          *top annotation
    |1   |p 6 7 8|
    |  3 |    8 9|
       *bot. annotation
\                    /
 \                  /
  \------page------/


## GENERAL DATA

### Score
- data[] 1,,-7'',p,space,endmessure,annotation,newline,newpage


### ScoreEditor
- add/del/change
- undo/redo

## GUI/DRAWING

### PageAnnotation
- TextItem[] (title, header, footer, pagenum, ...)

### PageAnnotationTemplate
- PageAnnotation[] (1st, odd, even)

### PageLayout (global)
- size
- margins (odd/even)
- portrait/landscape
- PageAnnotationTemplate

### LayoutSettings (per page)
-

### ScoreView
- Score*
- ScoreEditor*
- renderPage(QPainter*, int)
