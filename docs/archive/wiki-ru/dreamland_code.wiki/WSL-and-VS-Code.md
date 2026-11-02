# Сборка и работа в Windows

Дримленд собирается только под Ubuntu, однако и под Windows можно достичь вполне комфортной работы с помощью Windows Subsystem for Linux (WSL) и редактора Visual Studio Code. 

## Установка Windows Subsystem for Linux
Для установки WSL можно воспользоваться [официальной инструкцией Microsoft](https://docs.microsoft.com/en-us/windows/wsl/install-win10). Вкратце:

* Открыть Powershell от имени Администратора и выполнить команду:  
```powershell
Enable-WindowsOptionalFeature -Online -FeatureName Microsoft-Windows-Subsystem-Linux  
```
* Перезагрузить компьютер.  
* Скачать WSL Ubuntu 18.04 по адресу: https://aka.ms/wsl-ubuntu-1804. Лучше указать папку и имя сделать короче.  
(Например, C:\Ubuntu18.04onWindows.appx )  
* Установить скачанную программу, открыв Powershell от имени Администратора и выполнив: 
```powershell
Add-AppxPackage C:\Ubuntu18.04onWindows.appx  
```
* Запустить Ubuntu 18.04 LTS из меню Пуск.  
* Задать имя пользователя и пароль.  

## Подготовка окружения в Ubuntu

Установить все необходимые пакеты из секции ["Подготовка окружения"](https://github.com/dreamland-mud/dreamland_code/blob/master/README.md#env) основного README; собрать из исходников и запустить как описано там же.

> **Внимание!** Из-за [баги в WSL](https://github.com/microsoft/WSL/issues/658) при первой попытке запустить мир он может упасть. Эта бага исчезнет в очередном обновлении Windows 10 в мае 2020, также она исправлена в WSL2. Пока что можно временно обойти багу, удалив феневую базу данных командой `rm /home/dreamland/runtime/var/db/fenia/*`

## Установка Visual Studio Code

* Скачать последнюю версию [Visual Studio Code для Windows](https://code.visualstudio.com/updates), версия System.
* Запустить Visual Studio Code.
* Согласиться с предложением установить расширение "The Remote - WSL" 
![](https://dreamland.rocks/img/wsl1.png)

* Нажать на кнопку "Open a remote window" в нижнем левом углу.
* Выбрать Remote-WSL: New Window using Distro...
![](https://dreamland.rocks/img/wsl2.png)

* Выбрать Ubuntu-18.04 (default distro)
![](https://dreamland.rocks/img/wsl3.png)

* Открыть меню File и выбрать Open Folder...
![](https://dreamland.rocks/img/wsl4.png)

* Ввести путь до папки с кодом, в нашем случае /home/dreamland/dreamland_code/
* Установить все рекомендуемые расширения.
![](https://dreamland.rocks/img/wsl5.png)
