/* ==========================================================================
   主流程控制器：主菜单 + 关卡流程（对应原 Qt 的 MainWindow + GameController）
   ========================================================================== */

const App = {
  ctx: null,
  state: {
    name: "",
    level: 1,
    difficulty: 1,
    fragments: 0,
    time: 0,
  },
  _timer: null,
};

/* ---------- 计时（原 MainWindow::gameTimer，每 1000ms +1 秒） ---------- */
App.startTimer = function () {
  App.state.time = 0;
  if (App._timer) clearInterval(App._timer);
  App._timer = setInterval(function () { App.state.time++; }, 1000);
};
App.stopTimer = function () {
  if (App._timer) { clearInterval(App._timer); App._timer = null; }
};

/* ---------- 主菜单 ---------- */
App.menuScene = function () {
  const BTN = { x: 140, w: 200 };
  const items = [
    { name: "inputNameBtn", y: 260, h: 68, img: "startgame_name_button.png", on: App.onInputName },
    { name: "startGameBtn", y: 330, h: 65, img: "startgame_start_button.png", on: App.onStartGame },
    { name: "backgroundBtn", y: 400, h: 66, img: "startgame_background_button.png", on: App.onBackground },
    { name: "leaderboardBtn", y: 470, h: 65, img: "startgame_laber_button.png", on: App.onLeaderboard },
    { name: "exitBtn", y: 540, h: 63, img: "startgame_exit_button.png", on: App.onExit },
  ];
  let frame = 0, acc = 0, hover = -1;

  return {
    update: function (dt) {
      acc += dt;
      if (acc >= 0.45) { acc = 0; frame = (frame + 1) % 4; }   // 原版每 450ms 换一帧
    },
    draw: function (ctx) {
      G.draw(ctx, "background" + (frame + 1) + ".png", 0, 0, G.W, G.H);
      for (let i = 0; i < items.length; i++) {
        const b = items[i];
        G.draw(ctx, b.img, BTN.x, b.y, BTN.w, b.h);
        if (hover === i) {
          ctx.fillStyle = "rgba(255,255,255,0.16)";
          ctx.fillRect(BTN.x, b.y, BTN.w, b.h);
        }
      }
      // 当前昵称（主菜单背景是浅色调，用深棕色保证可读）
      const label = App.state.name ? ("当前昵称：" + App.state.name) : "尚未输入昵称";
      G.text(ctx, label, BTN.x, 235, 24, "#4a2c14");
    },
    onMouseMove: function (x, y) {
      hover = -1;
      for (let i = 0; i < items.length; i++) {
        const b = items[i];
        if (G.hit(x, y, [BTN.x, b.y, BTN.w, b.h])) { hover = i; break; }
      }
    },
    onMouseDown: function (x, y) {
      for (let i = 0; i < items.length; i++) {
        const b = items[i];
        if (G.hit(x, y, [BTN.x, b.y, BTN.w, b.h])) { G.playBtn(); b.on(); return; }
      }
    },
  };
};

/* ---------- 主菜单按钮行为 ---------- */
App.onInputName = function () {
  G.setScene(UI.nicknameScene(function (name) {
    if (name) App.state.name = name;
    G.setScene(App.menuScene());
  }));
};

App.onStartGame = function () {
  if (!App.state.name) {
    G.setScene(UI.nicknameScene(function (name) {
      if (!name) { G.setScene(App.menuScene()); return; }
      App.state.name = name;
      App.beginRun();
    }));
    return;
  }
  App.beginRun();
};

/* 重置进度并进入第 1 关的迷宫 */
App.beginRun = function () {
  App.state.level = 1;
  App.state.difficulty = 1;
  App.state.fragments = 0;
  App.startTimer();
  App.showMaze(App.state.level);
};

App.onBackground = function () {
  G.setScene(UI.backgroundScene(function () { G.setScene(App.menuScene()); }));
};

App.onLeaderboard = function () {
  G.setScene(UI.leaderboardScene(function () { G.setScene(App.menuScene()); }));
};

App.onExit = function () {
  App.stopTimer();
  G.setScene({
    draw: function (ctx) {
      ctx.fillStyle = "#000"; ctx.fillRect(0, 0, G.W, G.H);
      G.text(ctx, "感谢游玩 The Eighth（第八关）", G.W / 2, G.H / 2 - 30, 44, "#ffe9c9", "center");
      G.text(ctx, "可以关闭这个页面了", G.W / 2, G.H / 2 + 40, 26, "#b9b6cc", "center");
    },
  });
};

/* ---------- 迷宫（每一关都先走迷宫，出来后进入该关小游戏） ---------- */
App.showMaze = function (level) {
  G.setScene(Maze.create(level, function () {
    App.showLevelStart(App.state.level);
  }, function () {
    App.returnToMainMenu();
  }));
};

/* ---------- 关卡开始界面 ---------- */
App.LEVEL_START = {
  1: {
    w: 1201, h: 696, bg: "level1_startdialog.png",
    start: { x: 490, y: 420, w: 200, h: 60, img: "level1_startdialog_startbutton.png" },
    intro: { x: 490, y: 500, w: 200, h: 60, img: "level1_startdialog_introbutton.png" },
    back: { x: 490, y: 580, w: 200, h: 60, img: "level1_startdialog_returnbutton.png" },
  },
  2: {
    w: 1201, h: 676, bg: "level2_startdialog.png",
    start: { x: 490, y: 420, w: 200, h: 60, img: "level2_startdialog_startbutton.png" },
    intro: { x: 490, y: 500, w: 200, h: 60, img: "level2_startdialog_introbutton.png" },
    back: { x: 490, y: 570, w: 200, h: 67, img: "level2_startdialog_returnbutton.png" },
  },
  3: {
    w: 1201, h: 676, bg: "level3_startdialog.png",
    start: { x: 100, y: 360, w: 200, h: 65, img: "level3_startdialog_startbutton.png" },
    intro: { x: 100, y: 440, w: 200, h: 65, img: "level3_startdialog_introbutton.png" },
    back: { x: 100, y: 520, w: 200, h: 66, img: "level3_startdialog_returnbutton.png" },
  },
};

App.INTRO = {
  1: { w: 734, h: 1034, bg: "level1_intro_img.png", back: { x: 260, y: 890, w: 171, h: 151, img: "level1_intro_return_button.png" } },
  2: { w: 973, h: 706, bg: "level2_intro_img.png", back: { x: 410, y: 590, w: 161, h: 71, img: "level2_intro_return_button.png" } },
  3: { w: 861, h: 608, bg: "level3_intro_img.jpg", back: { x: 340, y: 520, w: 161, h: 61, img: "level3_intro_return_button.png" } },
};

App.showLevelStart = function (level) {
  const d = App.LEVEL_START[level];
  const scene = UI.dialog({
    w: d.w, h: d.h, bg: d.bg,
    buttons: [
      Object.assign({}, d.start, { on: function () { G.playBtn(); App.onLevelStart(level); } }),
      Object.assign({}, d.intro, { on: function () { G.playBtn(); App.showIntro(level); } }),
      Object.assign({}, d.back, { on: function () { G.playBtn(); App.returnToMainMenu(); } }),
    ],
  });
  scene.onKeyDown = function (k) { if (k === "Escape") App.returnToMainMenu(); };
  G.setScene(scene);
};

App.showIntro = function (level) {
  const d = App.INTRO[level];
  const scene = UI.dialog({
    w: d.w, h: d.h, bg: d.bg,
    buttons: [Object.assign({}, d.back, { on: function () { G.playBtn(); App.showLevelStart(level); } })],
  });
  scene.onKeyDown = function (k) { if (k === "Escape") App.showLevelStart(level); };
  G.setScene(scene);
};

/* ---------- 进入小游戏 ---------- */
App.onLevelStart = function (level) {
  const diff = App.state.difficulty;
  const callbacks = {
    onWin: App.onWin,
    onLose: App.onLose,
    onReturnToMenu: App.returnToMainMenu,
  };
  if (level === 1) G.setScene(Level1.create(diff, callbacks));
  else if (level === 2) G.setScene(Level2.create(diff, callbacks));
  else G.setScene(Level3.create(diff, callbacks));
};

App.onWin = function () {
  App.state.fragments++;
  App.handleGameResult(true, App.state.level, App.state.difficulty);
};

App.onLose = function () {
  App.handleGameResult(false, App.state.level, App.state.difficulty);
};

/* ---------- 胜负弹窗（坐标与原 Qt 对话框一一对应） ---------- */
App.RESULT = {
  "1-1": { w: 940, h: 621, bg: "l_1_1.png", up: [230, 450, 60, 70], next: [390, 460, 90, 50], menu: [570, 450, 100, 70] },
  "1-2": { w: 669, h: 621, bg: "l_1_2.png", up: [140, 510, 60, 40], next: [280, 510, 60, 40], menu: [410, 510, 80, 40] },
  "1-3": { w: 544, h: 626, bg: "l_1_3.png", next: [140, 520, 90, 40], menu: [300, 500, 120, 60] },
  "2-1": { w: 744, h: 701, bg: "l_2_1.png", up: [390, 450, 61, 41], next: [470, 450, 71, 41], menu: [560, 450, 71, 40] },
  "2-2": { w: 852, h: 653, bg: "l_2_2.png", up: [170, 440, 71, 51], next: [360, 450, 91, 40], menu: [540, 440, 91, 61] },
  "2-3": { w: 895, h: 854, bg: "l_2_3.png", next: [280, 410, 91, 40], menu: [490, 400, 91, 51] },
  "3-1": { w: 695, h: 524, bg: "l_3_1.png", up: [60, 350, 71, 41], next: [170, 350, 71, 41], menu: [280, 350, 81, 41] },
  "3-2": { w: 594, h: 630, bg: "l_3_2.png", up: [110, 440, 71, 51], next: [230, 440, 91, 51], menu: [370, 430, 101, 61] },
  "3-3": { w: 361, h: 661, bg: "l_3_3.png", next: [170, 360, 101, 51] },
};

App.handleGameResult = function (won, level, difficulty) {
  if (won) {
    const d = App.RESULT[level + "-" + difficulty];
    const buttons = [];
    if (d.up) buttons.push({ x: d.up[0], y: d.up[1], w: d.up[2], h: d.up[3], img: "next_level_button.png", on: function () { App.onGameResult(0); } });
    if (d.next) buttons.push({ x: d.next[0], y: d.next[1], w: d.next[2], h: d.next[3], img: "next_cart_button.png", on: function () { App.onGameResult(3); } });
    if (d.menu) buttons.push({ x: d.menu[0], y: d.menu[1], w: d.menu[2], h: d.menu[3], img: "main_window.png", on: function () { App.onGameResult(2); } });
    G.setScene(UI.dialog({ w: d.w, h: d.h, bg: d.bg, buttons: buttons }));
  } else {
    G.setScene(UI.dialog({
      w: 624, h: 482, bg: "fail_background.png",
      buttons: [
        { x: 20, y: 360, w: 161, h: 101, img: "fail_yes_button.png", label: "重新开始", size: 26, color: "#5a2418", on: function () { App.onGameResult(1); } },
        { x: 440, y: 10, w: 161, h: 101, img: "fail_yes_button.png", label: "←", size: 54, color: "#5a2418", on: function () { App.onGameResult(2); } },
      ],
    }));
  }
};

/* ---------- 结果分支（对应 GameController::onGameResult） ---------- */
App.onGameResult = function (result) {
  const level = App.state.level;
  const difficulty = App.state.difficulty;

  if (result === 0) {                       // 提高难度
    if (difficulty < 3) {
      App.state.difficulty = difficulty + 1;
      App.showLevelStart(level);
    } else if (level < 3) {
      App.state.level = level + 1;
      App.state.difficulty = 1;
      App.showMaze(App.state.level);
    } else {
      App.returnToMainMenu();
    }
  } else if (result === 1) {                // 重新开始
    App.onLevelStart(level);
  } else if (result === 2) {                // 返回主菜单
    if (level === 3) App.showFinal();
    else if (level === 2) App.showLevelStart(level);
    else App.returnToMainMenu();
  } else if (result === 3) {                // 进入下一关
    if (level < 3) {
      App.state.level = level + 1;
      App.state.difficulty = 1;
      App.showMaze(App.state.level);
    } else {
      App.showFinal();
    }
  }
};

/* ---------- 通关结束界面（原 FinalDialog：790x955） ---------- */
App.showFinal = function () {
  G.setScene(UI.dialog({
    w: 790, h: 955, bg: "final.png",
    buttons: [
      { x: 50, y: 800, w: 121, h: 121, img: "final_button.png", on: function () { App.returnToMainMenu(); } },
    ],
  }));
};

/* ---------- 回主菜单并保存排行榜 ---------- */
App.returnToMainMenu = function () {
  App.stopTimer();
  UI.saveScore(App.state.name, App.state.fragments, App.state.time);
  G.setScene(App.menuScene());
};

/* ---------- 启动 ---------- */
App.start = function (ctx) {
  App.ctx = ctx;
  G.setScene(App.menuScene());
};
