# Паскалеподобный интерпретатор

## Токены:

```c++
// Конструктивные элементы
'BEGIN' - начало блока
'END' - конец блока
'SEMI' ; -  признак конца оператора
'DOT' . - признак конца программы

// Управляющие элементы
'ASSIGN' - присвоить значение объекту слева

// Объекты
'INTEGER' - целое число
'ID' - Идентификатор

// Операнды
'PLUS' +
'MINUS' -
'MUL' *
'DIV' /

// Воспомогательные элементы
'LPAREN' (
'RPAREN' )

// Служебные элементы
'EOP' служебный токен, конец парсинга
```

## Грамматика

> `(expression)*` означает, что логика под скобочкой может быть повторена 0 и более раз

* Объявление переменной: `variable: ID`
* Фактор: `factor : PLUS factor | MINUS factor | INTEGER | LPAREN expr RPAREN | variable`
* Терм: `term: factor ((MUL | DIV) factor)*`
* Выражение: `expr: term ((PLUS | MINUS) term)*`
* Оператор присвоения: `assignment_statement : variable ASSIGN expr`
* Оператор: `statement : compound_statement | assignment_statement | empty`
* Список операторов: `statement_list : statement | statement SEMI statement_list`
* Блок операторов: `compound_statement : BEGIN statement_list END`
* Программа: `program : compound_statement DOT`
* Пустое место: `empty :`

# Работа интерпретатора

При интерпретировании создаётся глобальный Scope для переменных. В процессе чтения программы переменные создаются и получают значения, указанные в выражениях после оператора присваивания
В тесте проверяется что все переменные из этого окружения получили свои значения