# The Eighth（今天星期八）

基于 **Qt 5.9.9** 开发的横版闯关小游戏，包含三个玩法各异的关卡：

| 关卡 | 名称 | 玩法 |
|------|------|------|
| 第一关 | 红绸织梦 | 数字连线（Link Game） |
| 第二关 | 逃离梦魇 | 超级马里奥式平台跳跃 |
| 第三关 | 鬼神 | 空洞骑士式横版战斗 |

玩家先在迷宫中寻路，依次进入三个关卡，每通关一个难度获得一枚碎片，最多 9 枚碎片。

## 如何运行

### 方式一：直接下载可执行文件（推荐）

1. 打开本仓库的 **Releases** 页面
2. 下载 `The_eighth.exe`
3. 双击运行（仅支持 Windows）

> 所有图片、音乐资源已通过 `resources.qrc` 编译进 exe，**单个 exe 文件即可运行**，无需安装 Qt。

### 方式二：从源码编译

**环境要求**

- Qt 5.9.9（MinGW 5.3.0 32bit）
- Windows 系统

**编译步骤**

1. 打开 [build.bat](build.bat)，把 `QT_PATH` 改成你自己的 Qt 5.9.9 安装路径
2. 双击运行 `build.bat`
3. 编译完成后，可执行文件位于 `release/The_eighth.exe`

**手动编译**

```bash
# 将 Qt 的 bin 目录加入 PATH
set PATH=C:\Qt\Qt5.9.9\5.9.9\mingw53_32\bin;%PATH%

# 生成 Makefile
qmake The_eighth.pro -spec win32-g++ CONFIG+=release

# 编译
mingw32-make -j4
```

## 项目结构

```
The_eighth/
├── main.cpp                      # 程序入口
├── config.h                      # 全局配置（资源路径等）
├── mainwindow.h/cpp/ui           # 主窗口（开始界面）
├── gamecontroller.h/cpp          # 游戏总控制器（计时、碎片、流程）
├── mazewidget.h/cpp              # 迷宫
├── linkgamewidget.h/cpp          # 第一关：数字连线
├── mariowidget.h/cpp             # 第二关：超级马里奥
├── hollowknightwidget.h/cpp      # 第三关：空洞骑士
├── inputnamedialog.h/cpp/ui      # 输入昵称
├── backgrounddialog.h/cpp/ui     # 背景简介
├── leaderboarddialog.h/cpp/ui    # 排行榜
├── level1/2/3startdialog.h/cpp/ui# 各关卡开始界面
├── game1/2/3introdialog.h/cpp/ui # 各关卡玩法介绍
├── gameresultdialog.h/cpp/ui     # 游戏结算
├── result1_1 ~ result3_3dialog.* # 各关卡各难度的结果界面
├── finaldialog.h/cpp/ui          # 最终通关界面
├── resources.qrc                 # Qt 资源清单
├── images/                       # 图片资源
├── music/                        # 音频资源（背景音乐、按钮音效）
├── The_eighth.pro                # qmake 工程文件
├── build.bat                     # 一键编译脚本
└── README.md
```

## 游戏玩法

### 主菜单

- **输入昵称**：开始游戏前必须输入昵称
- **开始游戏**：进入迷宫，同时开始计时
- **背景简介**：查看游戏故事背景
- **排行榜**：查看玩家排名
- **退出游戏**：关闭游戏

### 迷宫

- 使用 `WASD` 或方向键移动
- 找到出口即可进入关卡开始界面
- 三个关卡的迷宫难度递增

### 第一关：数字连线

- 简单：6×6 网格，数字 1-5
- 中等：10×10 网格，数字 1-7
- 困难：12×12 网格，数字 1-10
- 点击数字开始连线，再点击空格子绘制路径
- 路径格子数 = 数字值 - 1
- 所有数字连接成对即为胜利

### 第二关：超级马里奥

- `WASD` 控制方向，空格键跳跃
- 收集金币、消灭敌人获得分数
- 得分超过 30 即为通关
- 敌人需要被打击 2 次才会死亡

### 第三关：空洞骑士

- `WASD` 移动，`J` 攻击，空格键飞行
- 击败 BOSS 即为通关
- 玩家血量显示在左下角（绿色），BOSS 血量显示在正上方（红色）

## 排行榜

排行榜数据保存在程序运行目录的 `leaderboard.txt` 中，格式为：

```
昵称,碎片数,完成时间(秒)
```

## 注意事项

1. Qt 版本必须为 5.9.9，不支持更高版本
2. 编译前确保 Qt 环境变量配置正确
3. 游戏计从点击「开始游戏」开始，第三关结束时停止
4. `release/`、`debug/`、`*.o`、`*.exe`、`*.pro.user` 等编译产物已在 `.gitignore` 中排除，克隆后需自行编译

## 许可证

本项目基于 MIT License 开源，详见 [LICENSE](LICENSE)。

> 注意：本项目依赖的 Qt 5.9.9 采用 LGPLv3 / GPLv3 授权，编译、分发时请一并遵守 Qt 的许可条款。
