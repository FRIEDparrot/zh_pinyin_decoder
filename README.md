## 中文拼音输入法

基于C语言的，面向 stm32, esp32, ARDUINO 等等嵌入式平台的，轻量级, 容易移植的中文拼音输入法。

目前支持拼音准确搜索和模糊匹配等功能， 之后会增加词库搜索支持

### 如何移植到你的嵌入式平台

- 此输入法全部需要的文件都在文件夹 zh_pinyin_decoder 下, 只需包含 zh_pinyin_decoder.h 即可, 目前测试平台为 windows, 只需稍加修改文件读取函数即可

> TODO : 之后会增加 stm32 平台的移植示例

### 版本更新日志

**V1.0 (2024.8.23):** 初次发布， 

**V1.1(2024.8.23)** : 修复 输入子串以 0 开头时的出错问题 

<b><mark style="background: transparent; color: red">V1.2(2024.8.25)</mark></b> : <b><mark style="background: transparent; color: red">添加了一种类似哈希表的新型拼音搜索算法作为可选的搜索算法, 该种算法可以在增加 2kb 左右ROM 的代价下, 提高搜索速度效率</mark></b>

>  如何使用哈希搜索算法: 只需要将 zh_pinyin_decoder.h  中的宏 `USE_ZH_HASH_BOOST` 设置为 1 即可  

**V1.3(2024.8.27) (当前版本)** : 

1. **修改了码表(zh_pinyin_decoder.h)中**的字顺序:   `好号浩豪耗郝昊皓毫灏嚎蒿濠蚝壕颢镐嗥薅貉嚆` 为 `嚆貉薅嗥镐颢壕蚝濠蒿嚎灏毫皓昊郝耗豪号好` 使其更符合实际词频。

2. 修改了部分头文件中的宏定义, 如 `MAX_PINYIN_BUFFER_SZ` 改为  `MAX_CODE_BUFFER_SZ`, `MAX_PINYIN_SEARCH_TYPES` 改为 `MAX_CODE_SEARCH_TYPES` 等。 

3. 添加了主要文件中的函数声明和 license 声明部分 

4. **增加了拼音分词功能**, 添加了相应的拼音分词示例, 同时修复了测试程序中空格导致字符分不同串的问题。

5. 修复了在测试文件中的 res_str 变量在程序结束时, 栈附近溢出的问题 (Run-Time Check Failure #2 - Stack around the variable 'res_str' was corrupted.)
