/* ==========================================================================
   第二关「逃离梦魇」横版跳跃模块
   （移植自 Qt 的 mariowidget.cpp / mariowidget.h）
   原版逻辑逐行对应，绘制坐标按逻辑画布 1201 x 676 直接使用。
   说明：原版为 QTimer 30ms 周期，这里用「累积 dt 到 30ms 再执行一次 updateGame」复刻。
   ========================================================================== */

const Level2 = {
  /* difficulty：难度 1/2/3（原 MarioWidget 构造函数第二个参数）
     cb：{ onWin, onLose, onReturnToMenu }
        onWin          ← 原 gameWon 信号
        onLose         ← 原 gameLost 信号
        onReturnToMenu ← 原 returnToMenu 信号（返回按钮 / ESC） */
  create: function (difficulty, cb) {

    /* ---------- 素材名（对应 config.h 的宏，均取 manifest.json 里的原始文件名） ---------- */
    const MARIO_B = "mario_background.png"; // MARIO_B（背景）
    const GUAI = "guai1.png";               // GUAI（怪兽）
    const ZHUAN = "zhuan.png";              // ZHUAN（砖：地面 / 返回按钮）
    const ZHUAN2 = "zhuan2.png";            // ZHUAN2（砖：障碍物）
    const BI1 = "bi1.png";                  // BI1（金币帧1）
    const BI2 = "bi2.png";                  // BI2（金币帧2）

    /* 玩家动画 4 帧（向左）：TUAN1_l / TUAN2_l / TUAN3_l / TUAN4_l
       注意 config.h 里 TUAN3_l 与 TUAN1_l 同为 tuan1.png */
    const PLAYER_L = ["tuan1.png", "tuan2.png", "tuan1.png", "tuan3.png"];
    /* 玩家动画 4 帧（向右）：TUAN1_r / TUAN2_r / TUAN3_r / TUAN4_r
       同理 TUAN3_r 与 TUAN1_r 同为 tuan1_1.png */
    const PLAYER_R = ["tuan1_1.png", "tuan2_1.png", "tuan1_1.png", "tuan3_1.png"];

    /* ---------- 关键常数（对应头文件成员 / movePlayer 里的常量） ---------- */
    const WORLD_WIDTH = 4000;        // WORLD_WIDTH
    const GROUND_Y = 550;            // GROUND_Y
    const PLAYER_SCREEN_X = 200;     // PLAYER_SCREEN_X（玩家始终固定在屏幕左侧 200px）
    const PLAYER_W = 51;             // m_playerWidth
    const PLAYER_H = 51;             // m_playerHeight
    const TICK = 0.03;               // 原 gameTimer 周期 30ms

    /* ---------- 状态变量（对应头文件成员，初值与原版构造函数一致） ---------- */
    let playerWorldX = 200;          // playerWorldX（地图坐标）
    let playerY = GROUND_Y - PLAYER_H; // playerY
    let playerVelocityX = 0;         // playerVelocityX
    let playerVelocityY = 0;         // playerVelocityY
    let isOnGround = false;          // isOnGround
    let jumpCount = 0;               // jumpCount（二段跳计数，最多 2）
    let invincibleCount = 2;         // invincibleCount（金身护体 2 次）
    let keyAttack = false;           // keyAttack（攻击键，松开立即取消）
    let lastDirection = 1;           // lastDirection（最后移动方向：1=右 -1=左）
    let facingDirection = 1;         // facingDirection（当前朝向，决定攻击方向）
    let score = 0;                   // score
    let coins = 0;                   // coins
    let enemiesKilled = 0;           // enemiesKilled
    let cameraX = 0;                 // cameraX
    let gameOver = false;            // m_gameOver（防止重复触发输赢）
    let keyLeft = false;             // m_keyLeft
    let keyRight = false;            // m_keyRight
    let coinFrame = 0;               // m_coinFrame（0/1）
    let coinFrameCounter = 0;        // m_coinFrameCounter
    let playerFrame = 0;             // m_playerFrame（0..3）
    let playerFrameCounter = 0;      // m_playerFrameCounter
    let hoverBack = false;           // 返回按钮 hover（代替 QPushButton:hover）

    let coinList = [];               // QList<Coin>
    let enemies = [];                // QList<Enemy>
    let platforms = [];              // QList<Platform>
    let obstacles = [];              // QList<Obstacle>

    let running = true;              // 代替 gameTimer->isActive()
    let accum = 0;                   // 周期累积器（秒）

    /* qrand() % n 的等价实现 */
    function rnd(n) { return Math.floor(Math.random() * n); }

    /* ========== 生成关卡里所有的东西：地面、障碍物、高台、金币、敌人 ========== */
    function initLevel() {
      // 清除
      coinList = [];
      enemies = [];
      platforms = [];
      obstacles = [];

      // ===== 地面：8段地面，段与段中间有空隙 =====
      const baseGap = 80.0;                                       // 地面之间的基本空隙宽度
      const groundWidths = [500, 500, 500, 500, 500, 400, 400, 250]; // 8段地面各自的宽度
      const groundCount = groundWidths.length;
      let currentX = 0;                                           // 当前这段地面从哪开始
      for (let i = 0; i < groundCount; i++) {
        platforms.push({ x: currentX, y: GROUND_Y, width: groundWidths[i], height: 50.0 });

        // 难度越高，地面之间的空隙越大
        let gapMult = 1.0;
        if (difficulty === 2) {
          gapMult = 1.0 + rnd(301) / 1000.0;                      // 中等：空隙放大 1.0~1.3 倍
        } else if (difficulty === 3) {
          gapMult = 1.2 + rnd(301) / 1000.0;                      // 困难：空隙放大 1.2~1.5 倍
        }
        currentX += groundWidths[i] + baseGap * gapMult;          // 下一段地面的起点
      }

      // ===== 障碍物：贴在地面上的方块，玩家得跳过去 =====
      const obstacleCount = (difficulty === 1) ? 2 : (difficulty === 2) ? 5 : 7; // 简单2/中等5/困难7
      const minSize = PLAYER_W;                                   // 最小跟玩家一样大（51）
      const maxSize = Math.floor(PLAYER_W * 1.5);                 // 最大是玩家的1.5倍（76）
      for (let i = 0; i < obstacleCount; i++) {
        const obsSize = minSize + rnd(maxSize - minSize + 1);     // 随机取 51~76
        let placed = false;
        // 试着放30次，找个不跟别的障碍物重叠的位置
        for (let attempt = 0; attempt < 30 && !placed; attempt++) {
          const p = platforms[rnd(groundCount)];                  // 随机挑一段地面
          const ox = p.x + rnd(Math.floor(p.width - obsSize));    // 这段地面范围内的随机横坐标
          const oy = p.y - obsSize;                               // 底边贴着地面
          let overlaps = false;
          for (let j = 0; j < obstacles.length; j++) {            // 跟已放好的比，四周各留20像素余量
            const o = obstacles[j];
            if (ox < (o.x + o.size) + 20 && (ox + obsSize) > o.x - 20 &&
                oy < (o.y + o.size) + 20 && (oy + obsSize) > o.y - 20) {
              overlaps = true; break;
            }
          }
          if (!overlaps) {
            obstacles.push({ x: ox, y: oy, size: obsSize });
            placed = true;
          }
        }
        if (!placed) {                                            // 试了30次还放不下，随便找个位置硬塞
          const p = platforms[rnd(groundCount)];
          obstacles.push({ x: p.x + rnd(Math.floor(p.width - obsSize)), y: p.y - obsSize, size: obsSize });
        }
      }

      // ===== 高空平台，玩家可以跳上去 =====
      for (let i = 0; i < 20; i++) {
        const px = 200 + rnd(WORLD_WIDTH - 400);                  // 横坐标：200~3799
        const py = 250 + rnd(200);                                // 纵坐标：250~449
        let overlap = false;
        for (let j = 0; j < platforms.length; j++) {
          const p = platforms[j];
          if (Math.abs(px - p.x) < 200 && Math.abs(py - p.y) < 100) { overlap = true; break; }
        }
        if (!overlap) platforms.push({ x: px, y: py, width: 60 + rnd(100), height: 20.0 }); // 宽60~159，高20
      }

      // ===== 撒金币：分数的主要来源 =====
      const coinCount = 10 + difficulty * 3;                      // 简单13/中等16/困难19
      for (let i = 0; i < coinCount; i++) {
        coinList.push({ x: 200 + rnd(WORLD_WIDTH - 400), y: 200 + rnd(300), collected: false });
      }

      // ===== 敌人：来回巡逻，血量2，得打两下才死 =====
      const enemyCount = 4 + difficulty * 2;                      // 简单6/中等8/困难10
      for (let i = 0; i < enemyCount; i++) {
        const ex = 300.0 + i * (WORLD_WIDTH / (enemyCount + 1));  // 均匀分布在整条路上
        enemies.push({
          x: ex, y: GROUND_Y - 40,                                // 站在地面上
          velocityX: (rnd(2) === 0) ? 2 : -2,                     // 随机朝右(+2)/朝左(-2)
          health: 2, alive: true,
        });
      }
    }

    /* ---------- 两个矩形是否相交（等价 QRect::intersects，左闭右开） ---------- */
    function overlap(ax, ay, aw, ah, bx, by, bw, bh) {
      return ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by;
    }

    /* ---------- 播放结束：胜利 / 失败（对应 emit gameWon / gameLost） ---------- */
    function win() {
      if (gameOver) return;
      gameOver = true;
      running = false;            // 代替 gameTimer->stop()
      cb.onWin();
    }
    function lose() {
      if (gameOver) return;
      gameOver = true;
      running = false;            // 代替 gameTimer->stop()
      cb.onLose();
    }

    /* ========== 每一帧更新玩家位置、处理各种碰撞（对应 movePlayer） ========== */
    function movePlayer() {
      // 速度控制
      const GRAVITY = 0.8;                                          // 每帧往下拉的重力加速度
      const MAX_FALL_SPEED = 15;                                    // 下落速度封顶，防止穿模
      const MOVE_SPEED = (difficulty === 1) ? 5.0 : (difficulty === 2) ? 6.5 : 8.0; // 难度越高跑得越快

      // ===== ① 左右移动：按哪个键往哪走，两个都按就保持上次方向 =====
      if (keyLeft && keyRight) {
        playerVelocityX = lastDirection * MOVE_SPEED;
        facingDirection = lastDirection;
      } else if (keyLeft) {
        playerVelocityX = -MOVE_SPEED;
        facingDirection = -1;
        lastDirection = -1;
      } else if (keyRight) {
        playerVelocityX = MOVE_SPEED;
        facingDirection = 1;
        lastDirection = 1;
      } else {
        playerVelocityX = 0;
      }

      // ===== ② 重力：每帧往下拽，但不能无限制加速 =====
      const prevBottom = playerY + PLAYER_H;                        // 移动前的脚底位置
      playerVelocityY += GRAVITY;
      if (playerVelocityY > MAX_FALL_SPEED) playerVelocityY = MAX_FALL_SPEED;

      playerWorldX += playerVelocityX;                              // 水平移动
      playerY += playerVelocityY;                                   // 垂直移动
      if (playerWorldX < 0) playerWorldX = 0;                       // 左边界
      if (playerWorldX > WORLD_WIDTH) playerWorldX = WORLD_WIDTH;   // 右边界

      // ===== ③ 站在平台上：脚底跟平台顶面接触时把玩家托住 =====
      isOnGround = false;
      for (let i = 0; i < platforms.length; i++) {
        const p = platforms[i];
        if (playerWorldX + PLAYER_W <= p.x || playerWorldX >= p.x + p.width) continue; // 水平不重叠
        if (playerVelocityY >= 0 &&
            prevBottom <= p.y + 15 &&
            playerY + PLAYER_H >= p.y - 5 &&
            playerY + PLAYER_H <= p.y + p.height + 10) {
          playerY = p.y - PLAYER_H;                                 // 拉到平台上面
          playerVelocityY = 0;
          isOnGround = true;
          jumpCount = 0;                                            // 重置跳跃次数
        }
      }

      // 掉到深渊里 → 游戏结束
      if (playerY > G.H + 100) { lose(); return; }

      // ===== ④ 跟障碍物碰撞：站上去或者被挡住 =====
      let prx = playerWorldX, pry = playerY, prw = PLAYER_W, prh = PLAYER_H;
      for (let i = 0; i < obstacles.length; i++) {
        const o = obstacles[i];

        // ④-a 站在障碍物上面（跟站平台同理）
        if (playerVelocityY >= 0 &&
            (prx + prw) > o.x + 3 &&                                // 水平确实重叠
            prx < (o.x + o.size) - 3) {
          const pBottom = playerY - playerVelocityY + PLAYER_H;     // 上一帧的脚底
          if (pBottom <= o.y + 10 &&
              playerY + PLAYER_H >= o.y - 3 &&
              playerY + PLAYER_H <= o.y + o.size * 0.5) {           // 脚没超过一半高度，防侧面误判
            playerY = o.y - PLAYER_H;
            playerVelocityY = 0;
            isOnGround = true;
            jumpCount = 0;
            pry = playerY;
            continue;
          }
        }

        // ④-b 水平方向被障碍物挡住（撞墙了）
        if ((prx + prw) > o.x && prx < (o.x + o.size) &&
            (pry + prh) > o.y + 5 && pry + PLAYER_H * 0.4 < (o.y + o.size)) {
          if (playerVelocityX > 0) {
            playerWorldX = o.x - PLAYER_W;                          // 推到障碍物左边
          } else if (playerVelocityX < 0) {
            playerWorldX = o.x + o.size;                            // 推到障碍物右边
          }
          playerVelocityX = 0;
          prx = playerWorldX;
        }
      }

      // ===== ⑤ 摄像机：玩家固定在屏幕左侧 200 像素 =====
      cameraX = playerWorldX - PLAYER_SCREEN_X;
      if (cameraX < 0) cameraX = 0;
      if (cameraX > WORLD_WIDTH - G.W) cameraX = WORLD_WIDTH - G.W;

      // ===== ⑥ 捡金币：碰到就算捡到，分数+1 =====
      for (let i = 0; i < coinList.length; i++) {
        const c = coinList[i];
        if (!c.collected) {
          if (overlap(prx, pry, prw, prh, c.x, c.y, 20, 20)) {
            c.collected = true;
            coins++;
            score++;
          }
        }
      }

      // ===== ⑦ 打敌人：按 J 攻击 or 踩踏 =====
      const keyAttackHeld = keyAttack;

      for (let i = 0; i < enemies.length; i++) {
        const e = enemies[i];
        if (!e.alive) continue;                                     // 死了的不管

        e.x += e.velocityX;                                         // 敌人巡逻移动
        if (e.x < 0 || e.x > WORLD_WIDTH - 30) e.velocityX *= -1;   // 碰边界掉头

        // ⑦-a 按 J 攻击：面朝方向 70 像素内、高度差小于 60 的敌人扣血
        if (keyAttackHeld) {
          const dx = (e.x + 15) - (playerWorldX + PLAYER_W / 2.0);
          const dy = (e.y + 15) - (playerY + PLAYER_H / 2.0);

          let inRange = false;
          if (facingDirection === 1 && dx > 0 && dx < 70) inRange = true;   // 朝右打右边
          if (facingDirection === -1 && dx < 0 && dx > -70) inRange = true; // 朝左打左边

          if (inRange && Math.abs(dy) < 60) {
            e.health--;                                             // 敌人掉血
            if (facingDirection === 1) e.x += 30;                   // 击退
            else e.x -= 30;

            if (e.health <= 0) {                                    // 打死了
              e.alive = false;
              enemiesKilled++;
              score += 2;
            }
            continue;
          }
        }

        // ⑦-b 碰到敌人：从上面踩→敌人扣血；从侧面撞→玩家扣「金身」次数
        if (overlap(prx, pry, prw, prh, e.x, e.y, 30, 30)) {
          if (playerVelocityY > 0 && playerY + PLAYER_H < e.y + 15) {   // 从上面踩
            e.health--;
            playerVelocityY = -10;                                  // 踩完弹起来
            if (e.health <= 0) {
              e.alive = false;
              enemiesKilled++;
              score += 2;
            }
          } else {
            // 金身护体：前 invincibleCount 次不杀死玩家
            if (invincibleCount > 0) {
              invincibleCount--;                                    // 消耗一次护体
              if (facingDirection === 1) e.x -= 60;                 // 把敌人推开
              else e.x += 60;
            } else {
              lose(); return;                                       // 护体用完，真的死了
            }
          }
        }
      }

      // ===== ⑧ 到达胜利分数 =====
      const targetScore = (difficulty === 1) ? 15 : (difficulty === 2) ? 20 : 30;
      if (score >= targetScore && !gameOver) win();
    }

    /* ========== 定时器回调（对应 updateGame）：动画帧 + 物理 ========== */
    function tick() {
      // 金币动画：每 0.2 秒切换帧（约 7 个 30ms 周期）
      coinFrameCounter++;
      if (coinFrameCounter >= 7) {
        coinFrameCounter = 0;
        coinFrame = (coinFrame + 1) % 2;
      }

      // 玩家走路动画：移动时每 0.15 秒切换帧（约 5 个 30ms 周期）
      if (playerVelocityX !== 0) {
        playerFrameCounter++;
        if (playerFrameCounter >= 5) {
          playerFrameCounter = 0;
          playerFrame = (playerFrame + 1) % 4;
        }
      } else {
        playerFrame = 0;
        playerFrameCounter = 0;
      }

      movePlayer();
    }

    /* ---------- 返回按钮（几何与原版 backBtn 一致：1090,0,98,59） ---------- */
    const BACK = [1090, 0, 98, 59];

    function drawBackButton(ctx) {
      // 原版 border-image: url(ZHUAN)
      if (!G.draw(ctx, ZHUAN, BACK[0], BACK[1], BACK[2], BACK[3])) {
        ctx.fillStyle = "darkgray";
        ctx.fillRect(BACK[0], BACK[1], BACK[2], BACK[3]);
      }
      if (hoverBack) {                                            // QPushButton:hover
        ctx.fillStyle = "rgba(255,255,255,0.4)";
        ctx.fillRect(BACK[0], BACK[1], BACK[2], BACK[3]);
      }
      // 原版：color: black; font-weight: bold
      G.text(ctx, "返回", BACK[0] + BACK[2] / 2, BACK[1] + BACK[3] / 2,
             20, "black", "center", "bold 20px " + G.FONT_SANS);
    }

    /* ---------- 初始化关卡 ---------- */
    initLevel();

    /* ---------- 场景对象 ---------- */
    return {
      update: function (dt) {
        if (!running) return;
        accum += dt;
        // 按原版 30ms 周期补执行；最多补 10 步，防止卡顿后追帧爆炸
        let guard = 0;
        while (accum >= TICK && guard < 10) {
          accum -= TICK;
          guard++;
          tick();
          if (!running) break;
        }
      },

      draw: function (ctx) {
        // 背景铺满整个窗口（缺图时填天空蓝，对应原版 QColor(135,206,235)）
        if (!G.draw(ctx, MARIO_B, 0, 0, G.W, G.H)) {
          ctx.fillStyle = "rgb(135,206,235)";
          ctx.fillRect(0, 0, G.W, G.H);
        }

        // 地面 / 平台
        for (let i = 0; i < platforms.length; i++) {
          const p = platforms[i];
          const sx = p.x - cameraX;
          if (sx + p.width < -50 || sx > G.W + 50) continue;
          if (!G.draw(ctx, ZHUAN, sx, p.y, p.width, p.height)) {
            ctx.fillStyle = "green";
            ctx.fillRect(sx, p.y, p.width, p.height);
          }
        }

        // 金币（2 帧交替）
        for (let i = 0; i < coinList.length; i++) {
          const c = coinList[i];
          const sx = c.x - cameraX;
          if (sx < -50 || sx > G.W + 50) continue;
          if (c.collected) continue;
          const coinImg = (coinFrame === 0) ? BI1 : BI2;
          if (!G.draw(ctx, coinImg, sx, c.y, 20, 20)) {
            ctx.fillStyle = "yellow";
            ctx.beginPath();
            ctx.arc(sx + 10, c.y + 10, 10, 0, Math.PI * 2);
            ctx.fill();
          }
        }

        // 敌人
        for (let i = 0; i < enemies.length; i++) {
          const e = enemies[i];
          const sx = e.x - cameraX;
          if (sx < -50 || sx > G.W + 50) continue;
          if (!e.alive) continue;
          if (!G.draw(ctx, GUAI, sx, e.y, 30, 30)) {
            ctx.fillStyle = "red";
            ctx.fillRect(sx, e.y, 30, 30);
          }
        }

        // 障碍物
        for (let i = 0; i < obstacles.length; i++) {
          const o = obstacles[i];
          const sx = o.x - cameraX;
          if (sx + o.size < -50 || sx > G.W + 50) continue;
          if (!G.draw(ctx, ZHUAN2, sx, o.y, o.size, o.size)) {
            ctx.fillStyle = "rgb(139,69,19)";                     // 棕色回退
            ctx.fillRect(sx, o.y, o.size, o.size);
          }
        }

        // 玩家动画：根据朝向选帧
        // 金身（invincibleCount>0）期间以透明度闪烁表现无敌状态
        const invincible = invincibleCount > 0 && !gameOver;
        if (invincible) ctx.globalAlpha = (Math.floor(G.time * 8) % 2 === 0) ? 1 : 0.35;
        const frames = (facingDirection === 1) ? PLAYER_R : PLAYER_L;
        if (!G.draw(ctx, frames[playerFrame], PLAYER_SCREEN_X, playerY, PLAYER_W, PLAYER_H)) {
          ctx.fillStyle = "blue";
          ctx.fillRect(PLAYER_SCREEN_X, playerY, PLAYER_W, PLAYER_H);
        }
        if (invincible) ctx.globalAlpha = 1;

        // 攻击特效（只要按住 J 就显示）
        if (keyAttack) {
          const ax = (facingDirection === 1)
            ? PLAYER_SCREEN_X + PLAYER_W / 2
            : PLAYER_SCREEN_X - 60 + PLAYER_W / 2;
          // 原版 drawEllipse(ax, playerY + H/2 - 30, 60, 60)：外接矩形左上角 (ax, cy)，center = (ax+30, cy+30)
          const cy = playerY + PLAYER_H / 2 - 30;
          ctx.save();
          ctx.fillStyle = "rgba(255,255,0,0.31)";                  // QColor(255,255,0,80)
          ctx.beginPath();
          ctx.arc(ax + 30, cy + 30, 30, 0, Math.PI * 2);
          ctx.fill();
          ctx.restore();
        }

        // HUD
        const targetScore = (difficulty === 1) ? 15 : (difficulty === 2) ? 20 : 30;
        const hudFont = "bold 20px " + G.FONT_SANS;                // pt12 bold
        G.text(ctx, "得分: " + score + " / " + targetScore, 10, 25, 20, "black", "left", hudFont);
        G.text(ctx, "金币: " + coins, 10, 65, 20, "black", "left", hudFont);
        G.text(ctx, "敌人: " + enemiesKilled, 10, 105, 20, "black", "left", hudFont);
        if (invincibleCount > 0)
          G.text(ctx, "金身: " + invincibleCount + " 次", 10, 145, 20, "black", "left", hudFont);

        // 返回按钮
        drawBackButton(ctx);
      },

      onKeyDown: function (k) {
        // ESC：原版为暂停切换，网页版无暂停 UI，统一改为返回主菜单
        if (k === "Escape") { cb.onReturnToMenu(); return; }

        if (k === "a" || k === "ArrowLeft") {
          keyLeft = true;
        } else if (k === "d" || k === "ArrowRight") {
          keyRight = true;
        } else if (k === "w" || k === "ArrowUp" || k === " ") {
          if (jumpCount < 2) {                                    // 二段跳
            playerVelocityY = -16;
            jumpCount++;
          }
        } else if (k === "j") {
          keyAttack = true;
        }
      },

      onKeyUp: function (k) {
        if (k === "a" || k === "ArrowLeft") keyLeft = false;
        else if (k === "d" || k === "ArrowRight") keyRight = false;
        else if (k === "j") keyAttack = false;                    // 释放立即取消攻击
      },

      onMouseMove: function (x, y) {
        hoverBack = G.hit(x, y, BACK);
      },

      onMouseDown: function (x, y) {
        if (G.hit(x, y, BACK)) cb.onReturnToMenu();
      },
    };
  },
};
