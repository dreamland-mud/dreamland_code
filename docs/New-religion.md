# Как добавить новую религию

## Добавьте конфигурацию для религии
В каталоге [dreamland_world/religions](https://github.com/dreamland-mud/dreamland_world/tree/master/religions) создайте новый XML файл с именем, соответствующим английскому названию новой религии. Можно скопировать один из существующих и отредактировать. Поля:

* тип религии в корневой ноде задает поведение татуировки, пока что укажите одно из существующих:
```xml
<Religion type="EnkiGod">
```
* ethos: список этосов, для которых эта религия доступна, если доступна всем - должно быть перечислено lawful neutral chaotic
* align: список характеров (alignments), для которых доступна, если доступна всем - перечислить evil neutral good
* races: список рас, если это поле пустое - доступна всем расам
* обновите description, nameRus, shortDescr и help согласно задумке


Добавьте эту религию в общую таблицу в [help religion](https://github.com/dreamland-mud/dreamland_world/blob/master/helps/religion.xml).

После этого в мире наберите:
```
plug reload religion
```
и новая религия станет доступна для выбора, а также появится в справке. 

## Опишите поведение татуировки

* В плагине [religion](https://github.com/dreamland-mud/dreamland_code/tree/master/plug-ins/religion)
добавьте объявление для класса нового бога в файл gods_impl.h. 
* Затем реализуйте метод tattooFight в gods_impl.cpp.
* Зарегистрируйте новый класс внутри impl.cpp. Всё можно делать по аналогии с существующими религиями.
* В конфигурации новой религии укажите название нового класса в аттрибуте type.







