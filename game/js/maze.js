/* ==========================================================================
   迷宫模块（移植自 Qt 的 mazewidget.cpp / mazewidget.h）
   原版逻辑逐行对应，绘制坐标按 gameSize()（= G.W x G.H）换算。
   ========================================================================== */

const Maze = {
  /* level：关卡号（原 MazeWidget 构造函数第二个参数）
     onComplete：走到出口时回调（原 mazeCompleted 信号）
     onReturnToMenu：按 ESC / 点「返回」时回调（原 returnToMenu 信号） */
  create: function (level, onComplete, onReturnToMenu) {

    /* ---------- 网格行列数：对应构造函数里的 switch(level) ---------- */
    let mazeWidth, mazeHeight;
    switch (level) {
      case 1: mazeWidth = 15; mazeHeight = 11; break;
      case 2: mazeWidth = 21; mazeHeight = 15; break;
      case 3: mazeWidth = 27; mazeHeight = 19; break;
      default: mazeWidth = 15; mazeHeight = 11; break;
    }

    /* ---------- 单个格子大小 ----------
       原版：cellSize = qMin(width() / mazeWidth, height() / mazeHeight)
       其中 width()/mazeWidth 是整数除法，这里用 Math.floor 保持一致 */
    const cellSize = Math.min(Math.floor(G.W / mazeWidth), Math.floor(G.H / mazeHeight));

    /* ---------- 素材名（对应 config.h 的宏） ---------- */
    const DOOR = "door.png";     // DOOR
    const WALL = "WALL.png";     // WALL
    const MIGONG = "migong.png"; // MIGONG（背景）

    /* 玩家动画 4 帧（向左）：TUAN1_l / TUAN2_l / TUAN3_l / TUAN4_l
       注意 config.h 里 TUAN3_l 与 TUAN1_l 同为 tuan1.png */
    const PLAYER_L = ["tuan1.png", "tuan2.png", "tuan1.png", "tuan3.png"];
    /* 玩家动画 4 帧（向右）：TUAN1_r / TUAN2_r / TUAN3_r / TUAN4_r
       同理 TUAN3_r 与 TUAN1_r 同为 tuan1_1.png */
    const PLAYER_R = ["tuan1_1.png", "tuan2_1.png", "tuan1_1.png", "tuan3_1.png"];

    /* ---------- 状态变量（对应头文件成员，初值与原版一致） ---------- */
    let maze = null;            // bool**：true=墙，false=通路
    let playerX = 1;            // 玩家出生点（列）
    let playerY = 1;            // 玩家出生点（行）
    let exitX = 0;              // 出口（列）
    let exitY = 0;              // 出口（行）
    let playerFrame = 0;        // m_playerFrame：当前动画帧 0..3
    let lastDirectionX = 1;     // m_lastDirectionX：1=右, -1=左
    let animAcc = 0;            // 帧动画累计时间（秒），代替 animationTimer
    let autoPath = [];          // m_autoPath：BFS 得到的路径（不含起点）
    let autoPathIndex = 0;      // m_autoPathIndex
    let autoPathRunning = false;// 代替 m_autoPathTimer->isActive()
    let autoAcc = 0;            // 自动寻路累计时间（秒），代替 m_autoPathTimer
    let hover = null;           // 按钮悬停："back" / "auto" / null

    /* ---------- 生成迷宫（DFS 深度优先生成树） ---------- */
    function generateMaze() {
      // 全部先填成墙；奇数行奇数列作为结点，墙厚一格
      maze = new Array(mazeHeight);
      for (let i = 0; i < mazeHeight; i++) {
        maze[i] = new Array(mazeWidth);
        for (let j = 0; j < mazeWidth; j++) maze[i][j] = true;
      }

      // 原版用 QStack 保存"当前路径"，qrand 选邻居
      const stack = [];
      maze[1][1] = false;      // 入口
      stack.push([1, 1]);

      while (stack.length > 0) {              // 循环直到所有格子都被访问过
        const cur = stack[stack.length - 1];  // 取栈顶（当前格子）
        const x = cur[0];                     // 当前列
        const y = cur[1];                     // 当前行

        // 收集所有可打通的邻居（间距为 2，未访问的格子值为 true）
        const neighbors = [];
        if (x > 2 && maze[y][x - 2]) neighbors.push([x - 2, y]);                    // 左邻
        if (x < mazeWidth - 3 && maze[y][x + 2]) neighbors.push([x + 2, y]);        // 右邻
        if (y > 2 && maze[y - 2][x]) neighbors.push([x, y - 2]);                    // 上邻
        if (y < mazeHeight - 3 && maze[y + 2][x]) neighbors.push([x, y + 2]);       // 下邻

        if (neighbors.length > 0) {                                     // 有未访问的邻居
          const idx = Math.floor(Math.random() * neighbors.length);     // qrand() % size
          const next = neighbors[idx];
          maze[(y + next[1]) / 2][(x + next[0]) / 2] = false;           // 打通中间的墙
          maze[next[1]][next[0]] = false;                              // 打通邻居格子本身
          stack.push(next);                                            // 邻居入栈，继续挖路
        } else {                                                       // 无可用邻居，回溯
          stack.pop();
        }
      }

      // 设置出口：右下角内部格子
      exitX = mazeWidth - 2;
      exitY = mazeHeight - 2;
      maze[exitY][exitX] = false;
    }

    /* ---------- 判断某格子是否能走（不越界且不是墙） ---------- */
    function isValidMove(x, y) {
      return (x >= 0 && x < mazeWidth && y >= 0 && y < mazeHeight) && !maze[y][x];
    }

    /* ---------- 自动寻路：BFS 求玩家到出口的最短路径 ---------- */
    function onAutoPath() {
      // 第一步：清理上次的自动寻路
      autoPathRunning = false;
      autoPath = [];
      autoPathIndex = 0;
      autoAcc = 0;

      // 第二步：BFS 从玩家当前位置搜索到出口
      const visited = [];                 // 标记格子是否已访问
      const parent = [];                  // 记录每个格子的前驱，用于回溯路径
      for (let i = 0; i < mazeHeight; i++) {
        visited.push(new Array(mazeWidth).fill(false));
        const row = new Array(mazeWidth);
        for (let j = 0; j < mazeWidth; j++) row[j] = [-1, -1];
        parent.push(row);
      }

      const q = [[playerX, playerY]];     // BFS 队列，起点为玩家当前位置
      visited[playerY][playerX] = true;
      let head = 0;                        // 队列头指针（代替 dequeue）

      const dx = [0, 0, -1, 1];            // 上、下、左、右
      const dy = [-1, 1, 0, 0];
      let found = false;

      while (head < q.length) {
        const cur = q[head++];
        const cx = cur[0], cy = cur[1];
        if (cx === exitX && cy === exitY) { found = true; break; }
        for (let i = 0; i < 4; i++) {
          const nx = cx + dx[i], ny = cy + dy[i];
          if (isValidMove(nx, ny) && !visited[ny][nx]) {
            visited[ny][nx] = true;
            parent[ny][nx] = cur;
            q.push([nx, ny]);
          }
        }
      }

      if (!found) return;                  // 理论上不可能找不到出口

      // 第三步：从出口沿 parent 回溯到起点，重建完整路径
      const path = [];                     // 只存起点之后的格子（含出口）
      let sx = exitX, sy = exitY;
      while (!(sx === playerX && sy === playerY)) {
        path.unshift([sx, sy]);
        const p = parent[sy][sx];
        sx = p[0];
        sy = p[1];
        if (sx === -1) break;              // 异常保护
      }
      autoPath = path;
      autoPathIndex = 0;

      // 第四步：启动自动行走（原版每 50ms 走一步）
      autoPathRunning = true;
      autoAcc = 0;
    }

    /* ---------- 自动行走一步（由 update 按 50ms 节奏触发） ---------- */
    function autoStep() {
      if (autoPathIndex >= autoPath.length) {
        autoPathRunning = false;
        return;
      }
      const next = autoPath[autoPathIndex];
      const nx = next[0], ny = next[1];

      // 根据移动方向更新朝向
      if (nx > playerX) lastDirectionX = 1;
      else if (nx < playerX) lastDirectionX = -1;

      playerX = nx;
      playerY = ny;
      autoPathIndex++;

      // 到达出口
      if (playerX === exitX && playerY === exitY) {
        autoPathRunning = false;
        onComplete();
      }
    }

    /* ---------- 方向键 / WASD 移动一步 ---------- */
    function handleMoveKey(k) {
      let nx = playerX, ny = playerY;
      switch (k) {
        case "w": case "ArrowUp": ny--; break;                       // 上移
        case "s": case "ArrowDown": ny++; break;                     // 下移
        case "a": case "ArrowLeft": nx--; lastDirectionX = -1; break;// 左移，朝左
        case "d": case "ArrowRight": nx++; lastDirectionX = 1; break;// 右移，朝右
        default: return;
      }
      if (isValidMove(nx, ny)) {
        playerX = nx;
        playerY = ny;
        // 到达出口
        if (playerX === exitX && playerY === exitY) onComplete();
      }
    }

    /* ---------- 右上角两个按钮（几何、文字与原版 QPushButton 一致） ---------- */
    const buttons = [
      { key: "back", x: 1050, y: 0, w: 100, h: 60, label: "返回" },
      { key: "auto", x: 1050, y: 65, w: 100, h: 60, label: "自动" },
    ];

    function drawButtons(ctx) {
      for (let i = 0; i < buttons.length; i++) {
        const b = buttons[i];
        // 原版按钮用 border-image: url(WALL)，即把墙图拉伸填满按钮
        if (!G.draw(ctx, WALL, b.x, b.y, b.w, b.h)) {
          ctx.fillStyle = "darkgray";
          ctx.fillRect(b.x, b.y, b.w, b.h);
        }
        if (hover === b.key) {                       // QPushButton:hover 效果
          ctx.fillStyle = "rgba(255,255,255,0.4)";
          ctx.fillRect(b.x, b.y, b.w, b.h);
        }
        // 原版：color: black; font-weight: bold
        ctx.save();
        ctx.font = "bold 20px " + G.FONT_SANS;
        ctx.fillStyle = "black";
        ctx.textAlign = "center";
        ctx.textBaseline = "middle";
        ctx.fillText(b.label, b.x + b.w / 2, b.y + b.h / 2);
        ctx.restore();
      }
    }

    /* ---------- 初始化 ---------- */
    generateMaze();

    /* ---------- 场景对象 ---------- */
    return {
      update: function (dt) {
        // 帧动画：原版 animationTimer 每 150ms 切换一帧
        animAcc += dt;
        while (animAcc >= 0.15) {
          animAcc -= 0.15;
          playerFrame = (playerFrame + 1) % 4;
        }
        // 自动寻路：原版 m_autoPathTimer 每 50ms 走一步
        if (autoPathRunning) {
          autoAcc += dt;
          while (autoAcc >= 0.05 && autoPathRunning) {
            autoAcc -= 0.05;
            autoStep();
          }
        }
      },

      draw: function (ctx) {
        // 背景铺满整个窗口（缺图时填黑，对应原版 fillRect(Qt::black)）
        if (!G.draw(ctx, MIGONG, 0, 0, G.W, G.H)) {
          ctx.fillStyle = "#000";
          ctx.fillRect(0, 0, G.W, G.H);
        }

        // 迷宫整体居中（原版整数除法）
        const ox = Math.floor((G.W - mazeWidth * cellSize) / 2);
        const oy = Math.floor((G.H - mazeHeight * cellSize) / 2);

        // 墙壁
        for (let y = 0; y < mazeHeight; y++) {
          for (let x = 0; x < mazeWidth; x++) {
            if (maze[y][x]) {
              const px = ox + x * cellSize;
              const py = oy + y * cellSize;
              if (!G.draw(ctx, WALL, px, py, cellSize, cellSize)) {
                ctx.fillStyle = "darkgray";
                ctx.fillRect(px, py, cellSize, cellSize);
              }
            }
          }
        }

        // 玩家（按朝向取左/右 4 帧；原版内缩 2px、尺寸 cellSize-4）
        const frames = (lastDirectionX >= 0) ? PLAYER_R : PLAYER_L;
        const ppx = ox + playerX * cellSize + 2;
        const ppy = oy + playerY * cellSize + 2;
        if (!G.draw(ctx, frames[playerFrame], ppx, ppy, cellSize - 4, cellSize - 4)) {
          ctx.fillStyle = "blue";
          ctx.fillRect(ppx, ppy, cellSize - 4, cellSize - 4);
        }

        // 出口（宽度放大到 1.1 倍；原版 doorW 为 int，截断取整）
        const doorW = Math.floor((cellSize - 4) * 1.1);
        const doorH = cellSize - 4;
        const epx = ox + exitX * cellSize + 2;
        const epy = oy + exitY * cellSize + 2;
        if (!G.draw(ctx, DOOR, epx, epy, doorW, doorH)) {
          ctx.fillStyle = "green";
          ctx.fillRect(epx, epy, doorW, doorH);
        }

        // 右上角按钮
        drawButtons(ctx);
      },

      onKeyDown: function (k) {
        // ESC：返回主菜单（原版为暂停切换，此处按导出接口约定改为返回）
        if (k === "Escape") { onReturnToMenu(); return; }
        handleMoveKey(k);
      },

      onMouseMove: function (x, y) {
        hover = null;
        for (let i = 0; i < buttons.length; i++) {
          const b = buttons[i];
          if (G.hit(x, y, [b.x, b.y, b.w, b.h])) { hover = b.key; break; }
        }
      },

      onMouseDown: function (x, y) {
        for (let i = 0; i < buttons.length; i++) {
          const b = buttons[i];
          if (G.hit(x, y, [b.x, b.y, b.w, b.h])) {
            if (b.key === "back") onReturnToMenu();
            else if (b.key === "auto") onAutoPath();
            return;
          }
        }
      },
    };
  },
};
