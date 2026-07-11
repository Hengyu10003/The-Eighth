# The Eighth - Game Project

基于Qt5.9.9开发的游戏项目，包含三个关卡：数字连连看、超级马里奥和空洞骑士。

## 项目结构

```
The eighth/
├── src/                    # 源代码目录
│   ├── main.cpp            # 主入口
│   ├── mainwindow.h/cpp    # 主窗口
│   ├── gamecontroller.h/cpp # 游戏控制器
│   ├── mazewidget.h/cpp    # 迷宫组件
│   ├── linkgamewidget.h/cpp # 数字连连看游戏
│   ├── mariowidget.h/cpp   # 超级马里奥游戏
│   ├── hollowknightwidget.h/cpp # 空洞骑士游戏
│   ├── inputnamedialog.h/cpp # 输入昵称对话框
│   ├── backgrounddialog.h/cpp # 背景简介对话框
│   ├── leaderboarddialog.h/cpp # 排行榜对话框
│   ├── levelstartdialog.h/cpp # 关卡开始对话框
│   ├── gameintrodialog.h/cpp # 游戏介绍对话框
│   └── gameresultdialog.h/cpp # 游戏结果对话框
├── resources/              # 资源目录
│   ├── resources.qrc       # 资源文件
│   └── images/             # 图片资源目录
├── The_eighth.pro         # Qt项目文件
├── build.bat              # 编译脚本
└── README.md              # 项目说明
```

## 编译运行

### 环境要求

- Qt 5.9.9（MinGW 5.3.0）
- Windows系统

### 编译步骤

1. 修改 `build.bat` 中的 `QT_PATH` 为你的Qt5.9.9安装路径
2. 双击运行 `build.bat`
3. 编译成功后，在 `release` 目录下找到 `The_eighth.exe`

### 手动编译

```bash
# 设置环境变量
set PATH=C:\Qt\Qt5.9.9\5.9.9\mingw53_32\bin;%PATH%

# 生成Makefile
qmake The_eighth.pro -spec win32-g++ CONFIG+=release

# 编译
mingw32-make -j4
```

## 游戏玩法

### 主菜单

- **输入昵称**：游戏前必须输入昵称
- **开始游戏**：进入迷宫，开始计时
- **背景简介**：查看游戏故事背景
- **排行榜**：查看玩家排名
- **退出游戏**：关闭游戏

### 迷宫系统

- 使用WASD或方向键控制移动
- 找到出口进入关卡开始界面
- 三个关卡的迷宫难度递增

### 关卡1：数字连连看

- 简单模式：6×6网格，数字1-5
- 中等模式：10×10网格，数字1-7
- 困难模式：12×12网格，数字1-10
- 点击数字开始连线，再点击空格子绘制路径
- 路径格子数 = 数字值 - 1
- 所有数字连接成对即为胜利

### 关卡2：超级马里奥

- WASD控制方向，空格键跳跃
- 收集金币和消灭敌人获得分数
- 得分超过30即为通关
- 敌人需要被打击2次才会死

### 关卡3：空洞骑士

- WASD控制移动，J键攻击，空格键飞行
- 击败BOSS即为通关
- 玩家血量在左下角（绿色）
- BOSS血量在正中间上方（红色）

## 贴图说明

在 `resources/images/` 目录下添加以下图片文件：

| 文件名 | 用途 |
|--------|------|
| background_story.png | 背景简介界面图片 |
| intro_linkgame.png | 数字连连看游戏介绍图片 |
| intro_mario.png | 超级马里奥游戏介绍图片 |
| intro_hollowknight.png | 空洞骑士游戏介绍图片 |
| game_result.png | 游戏结果界面图片 |

## 排行榜

排行榜数据保存在 `leaderboard.txt` 文件中，格式为：
```
昵称,碎片数,完成时间(秒)
```

## 注意事项

1. Qt版本必须为5.9.9，不支持更高版本
2. 编译前确保Qt环境变量正确配置
3. 游戏计时从点击"开始游戏"开始，关卡3结束时停止
4. 每个关卡的每个难度对应一个碎片，最多9个碎片