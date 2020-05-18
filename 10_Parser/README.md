# Паскалеподобный интерпретатор

## Токены:

```c++
// Конструктивные элементы
'PROGRAM' - начало программы
'VAR' - начало блока переменных
'BEGIN' - начало блока операторов
'END' - конец блока операторов
'SEMI' ; -  признак конца оператора
'DOT' . - признак конца программы

// Управляющие элементы
'ASSIGN' - присвоить значение объекту слева

// Типы
'INTEGER' - целое число
'REAL' - число с плавающей точкой

// Объекты
'INTEGER_CONST' - целое число
'REAL_CONST' - число с плавающей точкой
'ID' - Идентификатор

// Операнды
'PLUS' +
'MINUS' -
'MUL' *
'INTEGER_DIV ' / (целочисленное)
'FLOAT_DIV  ' / (с плавающей точкой)

// Воспомогательные элементы
'LPAREN' (
'RPAREN' )
'COLON' :
'COMMA' ,
'DOT' .

// Служебные элементы
'EOP' служебный токен, конец парсинга
```

## Грамматика

> `(expression)*` означает, что логика под скобочкой может быть повторена 0 и более раз

* Переменная: `variable: ID`
* Фактор: `factor : PLUS factor | MINUS factor | INTEGER_CONST | REAL_CONST | LPAREN expr RPAREN | variable`
* Терм: `term: factor ((MUL | INTEGER_DIV | FLOAT_DIV) factor)*`
* Выражение: `expr: term ((PLUS | MINUS) term)*`
* Оператор присвоения: `assignment_statement : variable ASSIGN expr`
* Оператор: `statement : compound_statement | assignment_statement | empty`
* Список операторов: `statement_list : statement | statement SEMI statement_list`
* Блок операторов: `compound_statement : BEGIN statement_list END`
* Тип переменной `type_spec : INTEGER | REAL`
* Объявление переменных: `variable_declaration : ID (COMMA ID)* COLON type_spec`
* Блок объявления переменных: `declarations : VAR (variable_declaration SEMI)+ | empty`
* Блок программы: `block : declarations compound_statement`
* Программа: `program : compound_statement DOT`
* Пустое место: `empty :`

## Работа интерпретатора

При интерпретировании создаётся глобальный Scope для переменных. В процессе чтения программы переменные создаются и получают значения, указанные в выражениях после оператора присваивания
В тесте проверяется что все переменные из этого окружения получили свои значения

## Проверка и запуск

для *nix систем: Makefile
```bash
# запустить тесты
$ make build_test 
$ make test 

# запустить valgrind для проверки на утечки
$ make memcheck
```

Для Windows - открыть task.sln (Создан в Visual Studio 2017), конфигурация Test. Проверки на утечки памяти уже "вставлены в код" при помощи содержимого `memcheck_crt.h` (Он может давать ложноположительные срабатывания, к сожалению)