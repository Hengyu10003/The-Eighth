/* ==========================================================================
   第三关「鬼神」Boss 战（移植自 Qt 的 hollowknightwidget.cpp / hollowknightwidget.h）
   逻辑画布 = gameSize() = G.W x G.H（1201 x 676），原版坐标直接沿用。
   原版 gameTimer 周期 50ms，网页版用「累积 dt 到 0.05s 阈值」复刻该步长。
   ========================================================================== */

const Level3 = {
  /* difficulty：难度（1/2/3）
     cb：{ onWin, onLose, onReturnToMenu } */
  create: function (difficulty, cb) {

    /* ---------- 素材名（对应 config.h 的宏） ---------- */
    /* 玩家 4 帧（向左）：TUAN1_l / TUAN2_l / TUAN3_l / TUAN4_l
       注意 config.h 里 TUAN3_l 与 TUAN1_l 同为 tuan1.png */
    const PLAYER_L = ["tuan1.png", "tuan2.png", "tuan1.png", "tuan3.png"];
    /* 玩家 4 帧（向右）：TUAN1_r / TUAN2_r / TUAN3_r / TUAN4_r
       同理 TUAN3_r 与 TUAN1_r 同为 tuan1_1.png */
    const PLAYER_R = ["tuan1_1.png", "tuan2_1.png", "tuan1_1.png", "tuan3_1.png"];
    /* 玩家子弹 2 帧：TAN_BI1 / TAN_BI2 */
    const BULLET = ["tan_bi1.png", "tan_bi2.png"];
    /* Boss 子弹 3 种贴图：BOSS_BI1 / BOSS_BI2 / BOSS_BI3 */
    const BOSS_BULLET = ["boss_bi1.png", "boss_bi2.png", "boss_bi3.png"];
    /* Boss 4 帧：BOSS1 ~ BOSS4 */
    const BOSS = ["boss1.png", "boss2.png", "boss3.png", "boss4.png"];

    /* 原版 gameTimer->start(50) 的步长（秒） */
    const TICK = 0.05;

    /* ---------- 状态变量（对应头文件成员，初值与原版一致） ---------- */
    let playerX = 100;              // playerX
    let playerY = 300;              // playerY
    let keyLeft = false, keyRight = false, keyUp = false, keyDown = false, keyShoot = false;
    let shootCooldown = 0;          // shootCooldown
    let lastDirectionX = 1;         // m_lastDirectionX：1=右, -1=左
    let playerFrame = 0;            // m_playerFrame
    let playerFrameCounter = 0;     // m_playerFrameCounter
    let bulletFrame = 0;            // m_bulletFrame
    let bossFrame = 0;              // m_bossFrame
    let bossFrameCounter = 0;       // m_bossFrameCounter
    let playerHealth = 200, playerMaxHealth = 200, playerAttack = 10;
    let bossX = 700, bossY = 250;
    let bossHealth = 100, bossMaxHealth = 100, bossAttack = 5;
    let bossMoveTimer = 0, bossShootTimer = 0;
    const playerBullets = [];       // PlayerBullet { x, y, vx, vy, active }
    const bossBullets = [];         // BossBullet  { x, y, vx, vy, active, tex }
    let tickAcc = 0;                // 累积时间，代替 gameTimer 的 50ms 周期
    let finished = false;           // 已判定胜负（原版靠切换界面终止，这里用标志位）

    /* ---------- 血量 / 攻击值：照搬构造函数 switch(difficulty) ---------- */
    switch (difficulty) {
      case 1:
        playerHealth = 200; playerMaxHealth = 200;
        bossHealth = 100; bossMaxHealth = 100;
        bossAttack = 5;
        break;
      case 2:
        playerHealth = 250; playerMaxHealth = 250;
        bossHealth = 100; bossMaxHealth = 100;
        bossAttack = 4;
        break;
      case 3:
        playerHealth = 300; playerMaxHealth = 300;
        bossHealth = 100; bossMaxHealth = 100;
        bossAttack = 3;
        break;
    }

    /* ---------- 右上角「返回」按钮（原版 backBtn：setGeometry(1150,0,50,30)） ---------- */
    const backBtn = { x: 1150, y: 0, w: 50, h: 30 };
    let backHover = false;

    /* ---------- 碰撞：对应 QRect::intersects（边贴边不算相交） ---------- */
    function collides(ax, ay, aw, ah, bx, by, bw, bh) {
      return ax < bx + bw && bx < ax + aw && ay < by + bh && by < ay + ah;
    }

    /* ===================== 玩家移动 + 射击（movePlayer） ===================== */
    function movePlayer() {
      const MOVE_SPEED = 5;

      // 移动
      if (keyLeft) playerX -= MOVE_SPEED;
      if (keyRight) playerX += MOVE_SPEED;
      if (keyUp) playerY -= MOVE_SPEED;
      if (keyDown) playerY += MOVE_SPEED;

      // 边界
      if (playerX < 0) playerX = 0;
      if (playerX > G.W - 60) playerX = G.W - 60;
      if (playerY < 0) playerY = 0;
      if (playerY > G.H - 60) playerY = G.H - 60;

      // ===== 根据按键确定射击方向 =====
      if (keyShoot && shootCooldown <= 0) {          // 按下射击键且冷却结束
        let vx = 0, vy = 0;
        if (keyLeft) vx = -1;
        if (keyRight) vx = 1;
        if (keyUp) vy = -1;
        if (keyDown) vy = 1;
        if (vx === 0 && vy === 0) { vx = 1; vy = 0; }// 没按方向键默认朝右
        const len = Math.sqrt(vx * vx + vy * vy);    // 归一化（对角线同速）
        if (len > 0) { vx /= len; vy /= len; }
        spawnPlayerBullet(vx * 10, vy * 10);         // 发速度
        shootCooldown = 12;                          // 冷却时间
      }
      if (shootCooldown > 0) shootCooldown--;        // 每帧减 1

      // ===== 玩家子弹移动 + 碰撞 =====
      for (let i = 0; i < playerBullets.length; i++) {
        const b = playerBullets[i];
        if (!b.active) continue;
        b.x += b.vx;
        b.y += b.vy;

        const bx = Math.trunc(b.x), by = Math.trunc(b.y);
        if (collides(bx, by, 30, 30, bossX, bossY, 110, 110)) {
          b.active = false;
          bossHealth -= playerAttack;
          if (bossHealth <= 0) {
            finished = true;
            cb.onWin();                              // 原版 emit gameWon()
            return;
          }
        }

        if (b.x < -50 || b.x > G.W + 50 || b.y < -50 || b.y > G.H + 50)
          b.active = false;
      }
    }

    /* ---------- 生成玩家子弹（spawnPlayerBullet） ---------- */
    function spawnPlayerBullet(vx, vy) {
      playerBullets.push({
        x: playerX + 30,
        y: playerY + 30,
        vx: vx,
        vy: vy,
        active: true,
      });
    }

    /* ===================== Boss 逻辑（updateBoss） ===================== */
    function updateBoss() {
      // 根据难度设置 Boss 移动速度
      const speedMult = (difficulty === 1) ? 1.0 : (difficulty === 2) ? 0.9 : 0.8;

      // Boss 左右移动（bossX 为 int，每帧截断）
      bossMoveTimer++;
      bossX = Math.trunc(bossX + Math.sin(bossMoveTimer * 0.03) * 3 * speedMult);

      // 边界
      if (bossX < 50) bossX = 50;
      if (bossX > G.W - 100) bossX = G.W - 100;

      // Boss 上下浮动（bossY 为 int）
      bossY = Math.trunc(250 + Math.sin(bossMoveTimer * 0.04) * 60);

      // Boss 射击
      bossShootTimer++;
      const shootInterval = (difficulty === 1) ? 50 : (difficulty === 2) ? 40 : 30;
      if (bossShootTimer >= shootInterval) {
        spawnBossBullet();
        bossShootTimer = 0;
      }

      // Boss 子弹移动 + 碰撞
      for (let i = 0; i < bossBullets.length; i++) {
        const b = bossBullets[i];
        if (!b.active) continue;
        b.x += b.vx;
        b.y += b.vy;

        const bx = Math.trunc(b.x), by = Math.trunc(b.y);
        if (collides(playerX, playerY, 60, 60, bx, by, 18, 18)) {
          b.active = false;
          playerHealth -= bossAttack;
          if (playerHealth <= 0) {
            finished = true;
            cb.onLose();                             // 原版 emit gameLost()
            return;
          }
        }
        // 边界之外消失
        if (b.x < -50 || b.x > G.W + 50 || b.y < -50 || b.y > G.H + 50)
          b.active = false;
      }
    }

    /* ---------- Boss 朝玩家方向射子弹（spawnBossBullet） ---------- */
    function spawnBossBullet() {
      const dx = playerX - bossX;             // 玩家在 Boss 右/左
      const dy = playerY - bossY;             // 玩家在 Boss 下/上
      const len = Math.sqrt(dx * dx + dy * dy);
      if (len === 0) return;                  // 贴脸了，不射
      const nx = dx / len;
      const ny = dy / len;

      const speed = 5;                                                                  // 子弹飞行速度
      const bulletCount = (difficulty === 1) ? 1 : (difficulty === 2) ? 2 : 3;          // 扇形子弹数

      for (let i = 0; i < bulletCount; i++) {
        // 多颗子弹时稍微散开角度
        const angle = Math.atan2(ny, nx) + (i - (bulletCount - 1) / 2.0) * 0.3;
        bossBullets.push({
          x: bossX + 25,                      // 从 Boss 中心射出
          y: bossY + 25,
          vx: Math.cos(angle) * speed,
          vy: Math.sin(angle) * speed,
          active: true,
          tex: Math.floor(Math.random() * 3), // qrand() % 3
        });
      }
    }

    /* ===================== 界面刷新（updateGame） ===================== */
    function updateGame() {
      // 玩家走路动画
      if (keyLeft || keyRight) {
        playerFrameCounter++;
        if (playerFrameCounter >= 3) {
          playerFrameCounter = 0;
          playerFrame = (playerFrame + 1) % 4;
        }
      } else {
        playerFrame = 0;
        playerFrameCounter = 0;
      }
      // 玩家子弹动画
      bulletFrame = (bulletFrame + 1) % 2;
      // Boss 动画
      bossFrameCounter++;
      if (bossFrameCounter >= 6) {
        bossFrameCounter = 0;
        bossFrame = (bossFrame + 1) % 4;
      }

      movePlayer();
      if (!finished) updateBoss();
    }

    /* ===================== 绘制（paintEvent） ===================== */
    function drawBars(ctx) {
      const cx = Math.floor(G.W / 2);

      // Boss 血条（顶部中间，红色）
      const bossBarWidth = Math.floor((bossHealth * 300) / bossMaxHealth);
      ctx.fillStyle = "#a9a9a9";                              // Qt::darkGray
      ctx.fillRect(cx - 150, 20, 300, 25);
      ctx.fillStyle = "red";
      ctx.fillRect(cx - 150, 20, bossBarWidth, 25);
      G.text(ctx, "BOSS: " + bossHealth + "/" + bossMaxHealth, cx - 140, 32, 15, "#fff", "left");

      // 玩家血条（Boss 血条正下方，绿色）
      const playerBarWidth = Math.floor((playerHealth * 200) / playerMaxHealth);
      ctx.fillStyle = "#a9a9a9";                              // Qt::darkGray
      ctx.fillRect(cx - 100, 55, 200, 20);
      ctx.fillStyle = "#00ff00";                              // Qt::green
      ctx.fillRect(cx - 100, 55, playerBarWidth, 20);
      G.text(ctx, "HP: " + playerHealth + "/" + playerMaxHealth, cx - 90, 65, 15, "#fff", "left");
    }

    /* 圆角矩形路径 */
    function roundRectPath(ctx, x, y, w, h, r) {
      ctx.beginPath();
      ctx.moveTo(x + r, y);
      ctx.lineTo(x + w - r, y);
      ctx.arcTo(x + w, y, x + w, y + r, r);
      ctx.lineTo(x + w, y + h - r);
      ctx.arcTo(x + w, y + h, x + w - r, y + h, r);
      ctx.lineTo(x + r, y + h);
      ctx.arcTo(x, y + h, x, y + h - r, r);
      ctx.lineTo(x, y + r);
      ctx.arcTo(x, y, x + r, y, r);
      ctx.closePath();
    }

    function drawBackButton(ctx) {
      const b = backBtn;
      ctx.save();
      roundRectPath(ctx, b.x, b.y, b.w, b.h, 5);
      // 原版 QSS：底 rgba(200,200,200,200)，hover 时 rgba(255,255,255,230)
      ctx.fillStyle = backHover ? "rgba(255,255,255,0.90)" : "rgba(200,200,200,0.78)";
      ctx.fill();
      ctx.lineWidth = 1;
      ctx.strokeStyle = "gray";
      ctx.stroke();
      ctx.restore();
      G.text(ctx, "返回", b.x + b.w / 2, b.y + b.h / 2, 15, "#000", "center", G.FONT_SANS);
    }

    function draw(ctx) {
      // 背景（原版 painter.fillRect(rect(), Qt::black)）
      ctx.fillStyle = "#000";
      ctx.fillRect(0, 0, G.W, G.H);

      // 玩家（60x60，按朝向选动画帧）
      const pframes = (lastDirectionX === -1) ? PLAYER_L : PLAYER_R;
      if (!G.draw(ctx, pframes[playerFrame], playerX, playerY, 60, 60)) {
        ctx.fillStyle = "#0000ff";                                  // Qt::blue
        ctx.fillRect(playerX, playerY, 60, 60);
      }

      // Boss（110×110，循环动画）
      if (!G.draw(ctx, BOSS[bossFrame], bossX, bossY, 110, 110)) {
        ctx.fillStyle = "#8b0000";                                  // Qt::darkRed
        ctx.fillRect(bossX, bossY, 110, 110);
      }

      // 玩家子弹（30×30，2 帧动画）
      for (let i = 0; i < playerBullets.length; i++) {
        const b = playerBullets[i];
        if (!b.active) continue;
        if (!G.draw(ctx, BULLET[bulletFrame], Math.trunc(b.x), Math.trunc(b.y), 30, 30)) {
          ctx.fillStyle = "#00ffff";                                // Qt::cyan
          ctx.fillRect(Math.trunc(b.x), Math.trunc(b.y), 30, 30);
        }
      }

      // Boss 子弹（18×18，随机贴图）
      for (let i = 0; i < bossBullets.length; i++) {
        const b = bossBullets[i];
        if (!b.active) continue;
        if (!G.draw(ctx, BOSS_BULLET[b.tex], Math.trunc(b.x), Math.trunc(b.y), 18, 18)) {
          ctx.fillStyle = "#808000";                                // Qt::darkYellow
          ctx.fillRect(Math.trunc(b.x), Math.trunc(b.y), 18, 18);
        }
      }

      // 血条
      drawBars(ctx);

      // 右上角返回按钮
      drawBackButton(ctx);
    }

    /* ===================== 场景对象 ===================== */
    return {
      enter: function () {
        // 清掉可能残留的按键状态，避免切场景后角色乱动
        keyLeft = keyRight = keyUp = keyDown = keyShoot = false;
        tickAcc = 0;
      },

      update: function (dt) {
        if (finished) return;
        // 按 50ms 周期执行一次 updateGame（复刻 gameTimer->start(50)）
        tickAcc += dt;
        while (tickAcc >= TICK) {
          tickAcc -= TICK;
          updateGame();
          if (finished) { tickAcc = 0; break; }
        }
      },

      draw: draw,

      /* 原版 keyPressEvent */
      onKeyDown: function (k) {
        // ESC：原版为暂停切换，网页版无暂停 UI，改为返回主菜单
        if (k === "Escape") { cb.onReturnToMenu(); return; }

        switch (k) {
          case "w": case "ArrowUp": keyUp = true; break;
          case "s": case "ArrowDown": keyDown = true; break;
          case "a": case "ArrowLeft": keyLeft = true; lastDirectionX = -1; break;
          case "d": case "ArrowRight": keyRight = true; lastDirectionX = 1; break;
          case "j": case " ": keyShoot = true; break;
        }
      },

      /* 原版 keyReleaseEvent（射击键释放立即取消） */
      onKeyUp: function (k) {
        switch (k) {
          case "w": case "ArrowUp": keyUp = false; break;
          case "s": case "ArrowDown": keyDown = false; break;
          case "a": case "ArrowLeft": keyLeft = false; break;
          case "d": case "ArrowRight": keyRight = false; break;
          case "j": case " ": keyShoot = false; break;
        }
      },

      onMouseMove: function (x, y) {
        backHover = G.hit(x, y, [backBtn.x, backBtn.y, backBtn.w, backBtn.h]);
      },

      onMouseDown: function (x, y) {
        if (G.hit(x, y, [backBtn.x, backBtn.y, backBtn.w, backBtn.h])) {
          cb.onReturnToMenu();
        }
      },
    };
  },
};
