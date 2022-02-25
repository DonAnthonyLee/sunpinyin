# 用 Visual Studio 编译

+ 在此文件夹下新建立子文件夹 libs
+ 将 sqlite 编译后的文件夹（需命名为 sqlite-build）移动到 libs 文件夹中
+ 打开 build/Win32/libsunpinyin.sln 项目即可进行编译

# 跨平台编译

## 在 Linux 中采用 MinGW64 编译

+ 编译运行时库（其中用户词频等将依赖 libsqlite3）
```bash
CC=x86_64-w64-mingw32-gcc CXX=x86_64-w64-mingw32-g++ LD=x86_64-w64-mingw32-ld PLATFORM=Win32 PKG_CONFIG_PATH=/opt/MinGW64/lib/pkgconfig scons --prefix=/MinGW64 --libdir=/MinGW64/lib --datadir=/MinGW64/share
```

+ 导出供其他程序使用的链接符号库
```bash
x86_64-w64-mingw32-dlltool --export-all --output-def libsunpinyin.def src/portability.os src/slm/slm.os src/lexicon/pytrie.os src/pinyin/pinyin_data.os src/pinyin/pinyin_seg.os src/pinyin/shuangpin_data.os src/pinyin/shuangpin_seg.os src/pinyin/hunpin_seg.os src/ime-core/imi_context.os src/ime-core/imi_data.os src/ime-core/lattice_states.os src/ime-core/imi_view.os src/ime-core/imi_uiobjects.os src/ime-core/imi_view_classic.os src/ime-core/imi_winHandler.os src/ime-core/ic_history.os src/ime-core/imi_funcobjs.os src/ime-core/imi_options.os src/ime-core/imi_option_event.os src/ime-core/userdict.os
sed -i "1d" libsunpinyin.def
x86_64-w64-mingw32-dlltool -D libsunpinyin.dll -d libsunpinyin.def -l libsunpinyin.a
```
