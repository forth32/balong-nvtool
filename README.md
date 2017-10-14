# balong-nvtool

## Russian

Утилита для обработки образов NVRAM устройств на чипсете Hisilicon Balong.

Данная утилита позволяет производить различные операции с образами NVRAM. В качестве образа можно использовать как файл с рабочей копией NVRAM (nv.bin), так и образы разделов NVRAM (nvdload, nvdefault, nvbacklte). Образ раздела nvimg для этих целей не подходит — это файловый раздел, из него надо предварительно извлечь файл nv.bin. Обычно внутри устройства рабочая копия nvram лежит в `/mnvm2:0/nv.bin`.

Утилита позволяет просматривать карту образа, извлекать из него отдельные или вся ячейки и компонентные файлы. Можно отредактировать любую ячейку на месте (в hex или символьном формате), можно загрузить образ ячейки из внешнего файла. Программа имеет в себе базу фирменных имен всех известных ячеек.  
Также программа умеет производить подбор OEM и SIMLOCK-кодов v4, соответствующих данному образу nvram.

В каталоге `modem-bin` лежат образы утилиты, предназначенные для работы внутри модема (под упралением работающего в модеме линукса). Эта версия утилиты может напрямую редактировать рабочую копию nvram (nv.bin) с корректным пересчетом контрольных сумм.

## English

Hisilicon Balong NVRAM image editing utility.

This utility can read and edit NVRAM images. Both NVRAM file (nv.bin) and NVRAM parititions (nvdload, nvdefault, nvbacklte) are supported. "nvimg" is a file partition with jffs/yaffs file system and is not suitale for editing directly. You should extract "nv.bin" from it first. This file is usually available inside the device as `/mnvm2:0/nv.bin`.

One can get NVRAM image contents, extract all or selected NVRAM items and component files. It's also possible to edit any NVRAM item or load it from external file. The program contains names for all known NVRAM items.  
There's also v4 OEM and SIMLOCK code brute force functionality.

`modem-bin` directory contains binary files for the on-device Linux. It can edit `/mnvm2:0/nv.bin` file directly and correctly recalculate all checksums.

This software is in Russian only. Use machine translation if needed.  
Please ask questions only about the program itself. No questions about devices, boot pins, loaders, NVRAM backups allowed.