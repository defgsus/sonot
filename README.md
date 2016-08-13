# Spanish Organ Notation Software

## VOCABULARY

    Messure
     /  \     Note
    /    \    |
    |1 2 |5 6 7 8| line \
    |2   |7''    | line  > ScoreLine
    |  4 |p   3  | line /
    \             /
     \-ScoreLine-/

          *top annotation
    |1   |p 6 7 8|
    |  3 |    8 9|
       *bot. annotation


## GENERAL DATA

### Messure
- data[] 1,, - 7'', p, space, annotation, newline, newpage

### ScoreLine
- Line[] of Messure[]

### Score
- ScoreLine[]
- pageIndex[]

### ScoreEditor
- add/del/change
- undo/redo


## GUI/DRAWING

### TextItem
- alignment
- font
- bounding-box
- margin

### PageAnnotation
- TextItem[] (title, header, footer, pagenum, ...)

### PageAnnotationTemplate
- PageAnnotation[] (1st, odd, even)

### PageSize
- width/height (mm)
- preset

### PageLayout
- PageSize
- page margins[odd/even]
- portrait/landscape
- PageAnnotationTemplate

### ScoreView
- Score*
- ScoreEditor*
- PageLayout (global)
- renderPage(QPainter*, int)
- optional per-page override:
    - PageMargins
    - PageAnnotation
