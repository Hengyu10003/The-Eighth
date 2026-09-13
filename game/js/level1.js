/* ==========================================================================
   第一关：红绸织梦（数字连线）
   移植自 Qt 的 linkgamewidget.cpp / linkgamewidget.h，逻辑逐行对应。
   逻辑画布 1201 x 676（与原版窗口 gameSize() 一致），原版坐标与尺寸可直接照搬。
   ========================================================================== */

const Level1 = {
  /* difficulty：难度 1~3（原 LinkGameWidget 构造函数第二个参数）
     cb：{ onWin, onLose, onReturnToMenu }（对应原 gameWon / gameLost / returnToMenu 信号） */
  create: function (difficulty, cb) {

    /* ---------- 难度参数：对应构造函数里的 switch (difficulty) ---------- */
    let gridSize, numRange;
    switch (difficulty) {
      case 1: gridSize = 6;  numRange = 5;  break;   // 6x6 棋盘，数字 1~5
      case 2: gridSize = 10; numRange = 7;  break;   // 10x10 棋盘，数字 1~7
      case 3: gridSize = 12; numRange = 10; break;   // 12x12 棋盘，数字 1~10
      default: gridSize = 6; numRange = 5;  break;
    }

    /* ---------- 素材名（对应 config.h 的宏） ---------- */
    const LINE = "line.png";         // LINE：背景

    /* ---------- 单个格子大小 ----------
       原版：cellSize = qMin(width() / gridSize, height() / gridSize) * 0.9
       两个除法都是整数除法，乘完 0.9 再截断赋给 int */
    const cellSize = Math.trunc(
      Math.min(Math.floor(G.W / gridSize), Math.floor(G.H / gridSize)) * 0.9
    );

    /* ---------- 颜色表（原版 colorsTable，RGB 原样照搬） ---------- */
    const colorsTable = [
      [231, 76, 60],     // 1 红
      [41, 128, 185],    // 2 蓝
      [39, 174, 96],     // 3 绿
      [230, 126, 34],    // 4 橙
      [142, 68, 173],    // 5 紫
      [22, 160, 133],    // 6 青
      [139, 90, 43],     // 7 棕
      [219, 60, 155],    // 8 粉
      [44, 62, 80],      // 9 深蓝
      [241, 196, 15],    // 10 黄
    ];

    /* ---------- 状态变量（对应头文件成员，初值与原版一致） ---------- */
    let grid = [];               // grid[行][列]：0=空，正数=未连的数字，负数=已连路径
    let colors = [];             // colors[行][列]：该格所属 pair 的颜色索引
    let totalPairs = 0;          // totalPairs
    let matchedPairs = 0;        // matchedPairs
    let isBuilding = false;      // m_isBuilding
    let buildStart = null;       // m_buildStart
    let buildPath = [];          // m_buildPath
    let pairs = [];              // m_pairs
    let connectionHistory = [];  // m_connectionHistory
    let hoverBack = false;       // 「返回」按钮的悬停状态（原 QPushButton:hover）

    /* ---------- 右上角「返回」按钮 ---------- */
    /* 原版：new QPushButton(this); setGeometry(1105, 0, 75, 45); setText("返回");
       样式表：background rgba(200,200,200,200)、1px 灰边、圆角 5，
       hover 时 background rgba(255,255,255,230)。原版按钮没有贴图，这里用同样的
       画布样式复刻（不臆造素材）。 */
    const backBtn = { x: 1105, y: 0, w: 75, h: 45 };

    /* ---------- 小工具 ---------- */
    function key(c, r) { return c + "," + r; }                   // 代替 QSet<QPoint> 的哈希
    function cellEq(a, b) { return a.x === b.x && a.y === b.y; }

    /* QColor::fromHsv(h, s, v)：h 0~359，s/v 0~255 */
    function hsvToRgb(h, s, v) {
      const hh = ((h % 360) + 360) % 360;
      const S = s / 255, V = v / 255;
      const c = V * S;
      const hp = hh / 60;
      const x = c * (1 - Math.abs((hp % 2) - 1));
      let r1 = 0, g1 = 0, b1 = 0;
      if (hp < 1) { r1 = c; g1 = x; }
      else if (hp < 2) { r1 = x; g1 = c; }
      else if (hp < 3) { g1 = c; b1 = x; }
      else if (hp < 4) { g1 = x; b1 = c; }
      else if (hp < 5) { r1 = x; b1 = c; }
      else { r1 = c; b1 = x; }
      const m = V - c;
      return [Math.round((r1 + m) * 255), Math.round((g1 + m) * 255), Math.round((b1 + m) * 255)];
    }

    /* QColor::darker(factor)：只在 HSV 的 V 上缩小，等价于各通道同乘 100/factor */
    function darker(rgb, factor) {
      return [
        Math.round(rgb[0] * 100 / factor),
        Math.round(rgb[1] * 100 / factor),
        Math.round(rgb[2] * 100 / factor),
      ];
    }

    function rgbStr(rgb, alpha) {
      return alpha === undefined
        ? "rgb(" + rgb[0] + "," + rgb[1] + "," + rgb[2] + ")"
        : "rgba(" + rgb[0] + "," + rgb[1] + "," + rgb[2] + "," + alpha + ")";
    }

    /* getColor(num)：数字不在 1~10 时返回 Qt::gray */
    function getColor(num) {
      if (num >= 1 && num <= 10) return colorsTable[num - 1];
      return [128, 128, 128];
    }

    /* 按 pair 颜色索引取色（对应原版 QColor::fromHsv(cidx*360/pairs.size(), 200, 255)） */
    function pairColor(cidx, num) {
      if (cidx >= 0) {
        const hue = Math.trunc(cidx * 360 / Math.max(1, pairs.length)) % 360;
        return hsvToRgb(hue, 200, 255);
      }
      return getColor(num);
    }

    /* 圆角矩形路径（原版 drawRoundedRect） */
    function roundRectPath(ctx, x, y, w, h, r) {
      const rr = Math.min(r, w / 2, h / 2);
      ctx.beginPath();
      ctx.moveTo(x + rr, y);
      ctx.lineTo(x + w - rr, y);
      ctx.arcTo(x + w, y, x + w, y + rr, rr);
      ctx.lineTo(x + w, y + h - rr);
      ctx.arcTo(x + w, y + h, x + w - rr, y + h, rr);
      ctx.lineTo(x + rr, y + h);
      ctx.arcTo(x, y + h, x, y + h - rr, rr);
      ctx.lineTo(x, y + rr);
      ctx.arcTo(x, y, x + rr, y, rr);
      ctx.closePath();
    }

    /* ===================== 棋盘生成（DFS 路径搜索） ===================== */

    /* 递归找路：从 pos 出发走 steps 步，落点为空位就算成功（对应 findBentPath） */
    function findBentPath(pos, steps, reserved, visited, outPath) {
      // 走到头了：步数走完，检查落点
      if (steps === 0) {
        if (!reserved.has(key(pos.x, pos.y))) {
          outPath.push(pos);
          return true;
        }
        return false;
      }

      // 还没走完：标记当前位置，继续探索
      visited.add(key(pos.x, pos.y));

      // 把上下左右四个方向打乱顺序
      const order = [0, 1, 2, 3];
      for (let i = 3; i > 0; i--) {
        const j = Math.floor(Math.random() * (i + 1));
        const t = order[i]; order[i] = order[j]; order[j] = t;
      }

      const dr = [-1, 1, 0, 0];   // 上、下、左、右 的行变化
      const dc = [0, 0, -1, 1];   // 上、下、左、右 的列变化

      for (let d = 0; d < 4; d++) {
        const nr = pos.y + dr[order[d]];
        const nc = pos.x + dc[order[d]];
        if (nc >= 0 && nc < gridSize && nr >= 0 && nr < gridSize &&
            !visited.has(key(nc, nr)) && !reserved.has(key(nc, nr))) {
          if (findBentPath({ x: nc, y: nr }, steps - 1, reserved, visited, outPath)) {
            outPath.unshift(pos);
            visited.delete(key(pos.x, pos.y));
            return true;
          }
        }
      }

      visited.delete(key(pos.x, pos.y));
      return false;
    }

    /* 生成数字连线棋盘（对应 generateBoard） */
    function generateBoard() {
      const reserved = new Set();    // 已经被占用的格子
      pairs = [];

      // 数字 1~numRange 各装一个"盒子"，从大到小排序
      const allPairs = [];
      for (let num = 1; num <= numRange; num++) allPairs.push(num);
      allPairs.sort(function (a, b) { return b - a; });

      for (let ai = 0; ai < allPairs.length; ai++) {
        const num = allPairs[ai];
        let placed = false;

        // 每个数字最多 120 次随机尝试
        for (let attempt = 0; attempt < 120; attempt++) {
          // 第一步：扫出所有空位
          const emptyCells = [];
          for (let r = 0; r < gridSize; r++)
            for (let c = 0; c < gridSize; c++)
              if (!reserved.has(key(c, r))) emptyCells.push({ x: c, y: r });

          // 空位不够 2 个 → 一对数字塞不下
          if (emptyCells.length < 2) break;

          // 第二步：随机选一个空位当起点
          const start = emptyCells[Math.floor(Math.random() * emptyCells.length)];

          // 第三步：DFS 找路（走 num 步，落到另一个空位）
          const foundPath = [];
          const visited = new Set();
          if (findBentPath(start, num, reserved, visited, foundPath)) {
            const end = foundPath[foundPath.length - 1];

            // 第四步：数字填到起点和终点
            grid[start.y][start.x] = num;
            grid[end.y][end.x] = num;

            // 第五步：整条路径上的格子标记为已占用
            for (let k = 0; k < foundPath.length; k++)
              reserved.add(key(foundPath[k].x, foundPath[k].y));

            // 第六步：记下这一对
            pairs.push({ number: num, p1: start, p2: end, path: [], connected: false });

            placed = true;
            break;
          }
        }
        if (!placed) return false;    // 120 次都放不下 → 生成失败
      }
      return true;
    }

    /* 初始化 / 重置棋盘（对应 initGame） */
    function initGame() {
      grid = [];
      colors = [];
      for (let i = 0; i < gridSize; i++) {
        grid.push(new Array(gridSize).fill(0));
        colors.push(new Array(gridSize).fill(0));
      }

      totalPairs = numRange;
      matchedPairs = 0;
      isBuilding = false;
      buildPath = [];
      pairs = [];
      connectionHistory = [];

      // 用 DFS 生成棋盘；万一失败，清空重试一次（与原版一致）
      if (!generateBoard()) {
        for (let i = 0; i < gridSize; i++)
          for (let j = 0; j < gridSize; j++) grid[i][j] = 0;
        pairs = [];
        generateBoard();
      }

      // 为每个 pair 分配颜色索引
      for (let pi = 0; pi < pairs.length; pi++) {
        const pair = pairs[pi];
        colors[pair.p1.y][pair.p1.x] = pi;
        colors[pair.p2.y][pair.p2.x] = pi;
      }
    }

    /* ===================== 辅助函数 ===================== */

    /* 计算点在棋盘中的位置（对应 cellAtPos，未命中返回 null） */
    function cellAtPos(x, y) {
      if (gridSize <= 0) return null;
      const offsetX = Math.floor((G.W - gridSize * cellSize) / 2);
      const offsetY = Math.floor((G.H - gridSize * cellSize) / 2);
      const c = Math.trunc((x - offsetX) / cellSize);   // C++ 整数除法向零取整
      const r = Math.trunc((y - offsetY) / cellSize);
      if (r >= 0 && r < gridSize && c >= 0 && c < gridSize) return { x: c, y: r };
      return null;
    }

    /* 两个格子是不是上下左右相邻 */
    function isAdjacent(a, b) {
      return (Math.abs(a.x - b.x) + Math.abs(a.y - b.y)) === 1;
    }

    function pathContains(path, cell) {
      for (let i = 0; i < path.length; i++)
        if (cellEq(path[i], cell)) return true;
      return false;
    }

    /* 所有数字对是否都连上了 */
    function checkWin() {
      for (let i = 0; i < pairs.length; i++)
        if (!pairs[i].connected) return false;
      return true;
    }

    /* 某格属于第几对未连上的数字（否则 -1） */
    function getPairIndex(cell) {
      for (let i = 0; i < pairs.length; i++) {
        const pair = pairs[i];
        if (!pair.connected && (cellEq(pair.p1, cell) || cellEq(pair.p2, cell)))
          return i;
      }
      return -1;
    }

    /* 玩家成功连上一对数字后的收尾（对应 connectPair） */
    function connectPair(idx) {
      if (idx < 0 || idx >= pairs.length) return;
      const pair = pairs[idx];

      // 整条路径填成负数，并涂上该 pair 的颜色
      for (let k = 0; k < pair.path.length; k++) {
        const p = pair.path[k];
        grid[p.y][p.x] = -pair.number;
        colors[p.y][p.x] = idx;
      }

      pair.connected = true;
      connectionHistory.push(idx);
      isBuilding = false;
      buildPath = [];
      matchedPairs++;

      // 全连完了 → 通关
      if (checkWin()) cb.onWin();
    }

    /* 点击处理（对应 handleLeftClick） */
    function handleLeftClick(cell) {
      const v = grid[cell.y][cell.x];

      // ===== 不在建造模式 =====
      if (!isBuilding) {
        // 点击已连接的数字 → 撤销该 pair
        if (v < 0) {
          for (let i = 0; i < pairs.length; i++) {
            const pair = pairs[i];
            if (!pair.connected) continue;
            if (!cellEq(pair.p1, cell) && !cellEq(pair.p2, cell)) continue;

            // 恢复数字
            grid[pair.p1.y][pair.p1.x] = pair.number;
            grid[pair.p2.y][pair.p2.x] = pair.number;
            // 清空路径格子（端点除外）
            for (let k = 0; k < pair.path.length; k++) {
              const p = pair.path[k];
              if (!cellEq(p, pair.p1) && !cellEq(p, pair.p2)) grid[p.y][p.x] = 0;
            }
            pair.connected = false;
            pair.path = [];
            const hi = connectionHistory.indexOf(i);
            if (hi >= 0) connectionHistory.splice(hi, 1);
            return;
          }
          return;
        }

        // 点击空格 → 忽略
        if (v === 0) return;

        // 点击未连接的数字 → 开始建造
        const pairIdx = getPairIndex(cell);
        if (pairIdx < 0) return;

        isBuilding = true;
        buildStart = { x: cell.x, y: cell.y };
        buildPath = [{ x: cell.x, y: cell.y }];
        return;
      }

      // ===== 正在建造模式 =====
      const last = buildPath[buildPath.length - 1];

      // 点击自身 / 已在路径中 / 不相邻 → 取消建造
      if (cellEq(cell, buildStart) || pathContains(buildPath, cell) || !isAdjacent(cell, last)) {
        isBuilding = false;
        buildPath = [];
        return;
      }

      const pathLen = buildPath.length;                          // 当前路径格子数（含起点）
      const startVal = grid[buildStart.y][buildStart.x];

      if (v > 0) {
        // 点到数字格子
        if (v === startVal && pathLen === v) {
          // 步数正确 → 连接
          buildPath.push({ x: cell.x, y: cell.y });

          let pairIdx = -1;
          for (let i = 0; i < pairs.length; i++) {
            const pair = pairs[i];
            if (pair.connected || pair.number !== v) continue;
            if ((cellEq(pair.p1, buildStart) && cellEq(pair.p2, cell)) ||
                (cellEq(pair.p2, buildStart) && cellEq(pair.p1, cell))) {
              pairIdx = i;
              break;
            }
          }

          if (pairIdx >= 0) {
            pairs[pairIdx].path = buildPath;
            connectPair(pairIdx);
          } else {
            // 两个数字同值但不属于同一对 → 取消
            isBuilding = false;
            buildPath = [];
          }
        } else {
          // 步数不对 / 不同数字 → 取消
          isBuilding = false;
          buildPath = [];
        }
        return;
      }

      // 点到已连的格子 → 取消
      if (v < 0) {
        isBuilding = false;
        buildPath = [];
        return;
      }

      // 空格 → 延伸路径（最多添加到 startVal 个格子）
      if (pathLen < startVal) buildPath.push({ x: cell.x, y: cell.y });
    }

    /* ===================== 绘图 ===================== */

    function draw(ctx) {
      // 背景（原版 m_bgPixmap = LINE，铺满整个窗口）
      if (!G.draw(ctx, LINE, 0, 0, G.W, G.H)) {
        ctx.fillStyle = "#ecf0f1";
        ctx.fillRect(0, 0, G.W, G.H);
      }

      if (gridSize <= 0) return;

      // 棋盘居中偏移（原版整数除法）
      const offsetX = Math.floor((G.W - gridSize * cellSize) / 2);
      const offsetY = Math.floor((G.H - gridSize * cellSize) / 2);

      // 格子矩形：内缩 1 像素，宽高各少 2 像素
      function cellRect(r, c) {
        return {
          x: offsetX + c * cellSize + 1,
          y: offsetY + r * cellSize + 1,
          w: cellSize - 2,
          h: cellSize - 2,
        };
      }

      // 原版 QFont：pointSize = max(8, cellSize / 2)，粗体；点值换算成像素（96 DPI）
      const numPx = Math.round(Math.max(8, Math.trunc(cellSize / 2)) * 4 / 3);

      ctx.save();
      ctx.lineJoin = "miter";

      // 逐格绘制
      for (let i = 0; i < gridSize; i++) {          // i = 行号
        for (let j = 0; j < gridSize; j++) {        // j = 列号
          const rct = cellRect(i, j);
          const v = grid[i][j];
          const cidx = colors[i][j];

          if (v === 0) {
            // 情况1：空格子 → 白色方框（不填充）
            ctx.strokeStyle = "#ffffff";
            ctx.lineWidth = 1;
            ctx.strokeRect(rct.x, rct.y, rct.w, rct.h);
          } else if (v > 0) {
            // 情况2：未连上的数字格子 → 彩色圆角边框 + 数字
            const baseColor = pairColor(cidx, v);
            ctx.strokeStyle = rgbStr(darker(baseColor, 110));
            ctx.lineWidth = 2;
            roundRectPath(ctx, rct.x + 1, rct.y + 1, rct.w - 2, rct.h - 2, 4);
            ctx.stroke();

            ctx.font = "bold " + numPx + "px " + G.FONT_SANS;
            ctx.fillStyle = rgbStr(baseColor);
            ctx.textAlign = "center";
            ctx.textBaseline = "middle";
            ctx.fillText(String(v), rct.x + rct.w / 2, rct.y + rct.h / 2);
          } else {
            // 情况3：已连好的路径格子 → 实心填色 + 深色细边框
            const n = -v;
            const baseColor = pairColor(cidx, n);

            ctx.fillStyle = rgbStr(baseColor);
            ctx.fillRect(rct.x, rct.y, rct.w, rct.h);
            ctx.strokeStyle = rgbStr(darker(baseColor, 130));
            ctx.lineWidth = 1;
            ctx.strokeRect(rct.x, rct.y, rct.w, rct.h);

            // 只有端点才写数字（白色）
            let isEndpoint = false;
            for (let pi = 0; pi < pairs.length; pi++) {
              const pair = pairs[pi];
              if (pair.connected) {
                if ((pair.p1.x === j && pair.p1.y === i) ||
                    (pair.p2.x === j && pair.p2.y === i)) { isEndpoint = true; break; }
              }
            }
            if (isEndpoint) {
              ctx.font = "bold " + numPx + "px " + G.FONT_SANS;
              ctx.fillStyle = "#ffffff";
              ctx.textAlign = "center";
              ctx.textBaseline = "middle";
              ctx.fillText(String(n), rct.x + rct.w / 2, rct.y + rct.h / 2);
            }
          }
        }
      }

      // 建造路径
      if (isBuilding && buildPath.length > 0) {
        let n = Math.abs(grid[buildStart.y][buildStart.x]);
        if (n === 0) n = 1;
        const cidx = colors[buildStart.y][buildStart.x];
        const pathColor = (cidx >= 0)
          ? hsvToRgb(Math.trunc(cidx * 360 / Math.max(1, pairs.length)) % 360, 255, 255)
          : getColor(n);

        // 路径格子中间的半透明小方块
        ctx.fillStyle = rgbStr(pathColor, 80 / 255);
        for (let k = 1; k < buildPath.length; k++) {
          const p = buildPath[k];
          const r0 = cellRect(p.y, p.x);
          ctx.fillRect(r0.x + 3, r0.y + 3, r0.w - 6, r0.h - 6);
        }

        // 路径格子之间的虚线
        ctx.strokeStyle = rgbStr(pathColor);
        ctx.lineWidth = 3;
        ctx.setLineDash([12, 6]);
        ctx.beginPath();
        for (let k = 1; k < buildPath.length; k++) {
          const a = buildPath[k - 1], b = buildPath[k];
          ctx.moveTo(offsetX + a.x * cellSize + cellSize / 2, offsetY + a.y * cellSize + cellSize / 2);
          ctx.lineTo(offsetX + b.x * cellSize + cellSize / 2, offsetY + b.y * cellSize + cellSize / 2);
        }
        ctx.stroke();
        ctx.setLineDash([]);

        // 起点金色高亮边框
        const s0 = cellRect(buildStart.y, buildStart.x);
        ctx.strokeStyle = "rgb(241,196,15)";
        ctx.lineWidth = 3;
        roundRectPath(ctx, s0.x + 2, s0.y + 2, s0.w - 4, s0.h - 4, 4);
        ctx.stroke();
      }

      ctx.restore();

      // 右上角「返回」按钮
      ctx.save();
      roundRectPath(ctx, backBtn.x, backBtn.y, backBtn.w, backBtn.h, 5);
      ctx.fillStyle = hoverBack ? "rgba(255,255,255,0.90)" : "rgba(200,200,200,0.78)";
      ctx.fill();
      ctx.strokeStyle = "#808080";
      ctx.lineWidth = 1;
      ctx.stroke();
      ctx.restore();
      G.text(ctx, "返回", backBtn.x + backBtn.w / 2, backBtn.y + backBtn.h / 2, 16, "#000000", "center", G.FONT_SANS);
    }

    /* ---------- 初始化（对应构造函数里的 initGame） ---------- */
    initGame();

    /* ---------- 场景对象 ---------- */
    return {
      draw: draw,

      /* ESC：原版是暂停切换；网页版没有暂停 UI，统一改为返回主菜单 */
      onKeyDown: function (k) {
        if (k === "Escape") cb.onReturnToMenu();
      },

      onMouseMove: function (x, y) {
        hoverBack = G.hit(x, y, [backBtn.x, backBtn.y, backBtn.w, backBtn.h]);
      },

      onMouseDown: function (x, y) {
        // 返回按钮（原版是 QPushButton 的点击）
        if (G.hit(x, y, [backBtn.x, backBtn.y, backBtn.w, backBtn.h])) {
          G.playBtn();
          cb.onReturnToMenu();
          return;
        }
        // 棋盘点击（对应 mousePressEvent 的左键分支）
        const cell = cellAtPos(x, y);
        if (!cell) return;
        handleLeftClick(cell);
      },
    };
  },
};
