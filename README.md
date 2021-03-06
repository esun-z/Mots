# Mots

An in-game players' scores monitor for Wows-CN.

战舰世界国服局内玩家战绩监听工具。

## 功能

- [x] 局内所有玩家总体战绩查询
- [x] 局内所有玩家在当前驾驶的船只上的战绩查询
- [x] 计算局内所有玩家在当前船只上的 Personal Rating 值
- [x] 根据 Personal Rating 值进行分颜色显示和排序

## 特点

- [x] 更低的内存和处理器资源占用
- [x] 相对更快的查询速度

## 使用

在 [Release](https://github.com/esun-z/Mots/releases) 中下载一个可执行文件，并使用。

阅读玩家数据：

| 行   | 标题                         | 内容                                                         |
| ---- | ---------------------------- | ------------------------------------------------------------ |
| 1    | 基础信息                     | 玩家名  [船只]  AvgTier: 玩家游玩的船只平均等级  PR: Personal Rating 值 |
| 2    | 战绩总览（Overview）         | 总场数  胜率  场均经验  场均伤害  场均击沉                   |
| 3    | 当前船只战绩（Current Ship） | 总场数  胜率  场均经验  场均伤害  场均击沉                   |

您也可以随时阅读“帮助”选项卡中的内容以获取更多信息。

## 使用的框架、库与接口

- Qt
- 来自[@西行寺雨季](https://shinoaki.com/)的 PR 平均值接口

## 开发环境

MSVC, Visual Studio 2022 with Qt 5.15.2_msvc2019_64

## 合法性问题

在 https://wowsgame.cn/robots.txt 中写道，从 https://wowsgame.cn/community/ 爬取信息是被允许的。

## 鸣谢

感谢[@西行寺雨季](https://github.com/JustOneSummer)的指导和支持！