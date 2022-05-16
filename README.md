# Описание структуры проекта
* Файлы в корне - рабочая версия кода, без дополнительных комментариев;
* VersionWithComments/ - директория с аналогичной версией кода, дополненной комментариями, логами для отслеживания шагов алгоритма;
* TinyXml/ - вспомогательная библиотека для парсинга xml-файлов;
* InitialTests/ - директория с тестами и их результатами; 
* TestGeneration/ - директория с кодом для генерации тестов.

# Запуск тестов

1. Соберите программу: в корневой директории или в директории VersionWithComments/:
```
make -B
```
2. Запустите тест:
```
./main <путь файла со входными данными> <путь до файла с результатами> <глубина перебора> <число итераций возврата>
```
Здесь:
* файл со входными данными - файл в xml-формате, описанном в InitialTests/format.xml
* файл с результатами - файл в .dot-формате, куда записать построенное расписание
* глубина перебора - SearchDepth из ВКР
* число итераций возврата - IterationDepth из ВКР 

Результат выполнения выводится на стандартный вывод:
* 0 - алгоритм вернул неуспех
* 1 - алгоритм построил корректное расписание

# Какие тесты можно запустить?

* InitialTests/Inputs/XML_Test_1.xml
* InitialTests/Inputs/XML_Test_2.xml
* InitialTests/Inputs/XML_Test_3.xml
* InitialTests/Inputs/XML_Test_tiny.xml

Результаты адекватно интерпретируются глазами, для этого можно .dot-файлы посмотреть тут: http://magjac.com/graphviz-visual-editor/

Сгенерированные тесты, более сложные для восприятия:

* InitialTests/Inputs/Generated/100_6.xml
* InitialTests/Inputs/Generated/40_6.xml
* InitialTests/Inputs/Generated/50_1.xml
* InitialTests/Inputs/Generated/70_1.xml
* InitialTests/Inputs/Generated/80_3.xml
* InitialTests/Inputs/Generated/90_7.xml
  
