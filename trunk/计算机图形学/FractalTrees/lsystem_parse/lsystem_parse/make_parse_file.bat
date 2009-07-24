@echo off
del expression.yy.c
del expression_tab.h
del expression_tab.c
flex expression.l
rename lex.yy.c expression.yy.c
bison -dv expression.y
