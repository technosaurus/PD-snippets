// Data Models Engine Schema
const tasks = [
    { id: "t1", type: "task",      name: "Market Research", start: "2026-07-20", end: "2026-07-23", progress: 1.00, color: "#10b981" },
    { id: "t2", type: "task",      name: "Scoping & Specs", start: "2026-07-23", end: "2026-07-26", progress: 0.85, color: "#10b981", dependencies: ["t1"] },
    { id: "m1", type: "milestone", name: "Scope Sign-off",  start: "2026-07-27", color: "#ef4444", dependencies: ["t2"] },
    { id: "t3", type: "task",      name: "UI Architecture",  start: "2026-07-27", end: "2026-07-31", progress: 0.40, color: "#3b82f6", dependencies: ["m1"] },
    { id: "t4", type: "task",      name: "Frontend Dev",    start: "2026-08-01", end: "2026-08-08", progress: 0.10, color: "#8b5cf6", dependencies: ["t3"] },
    { id: "t5", type: "task",      name: "Backend Dev",     start: "2026-08-03", end: "2026-08-10", progress: 0.00, color: "#8b5cf6", dependencies: ["m1"] },
    { id: "t6", type: "task",      name: "Integration & QA",start: "2026-08-11", end: "2026-08-15", progress: 0.00, color: "#f59e0b", dependencies: ["t4", "t5"] },
    { id: "m2", type: "milestone", name: "Production Launch",start: "2026-08-16", color: "#ef4444", dependencies: ["t6"] }
];

const config = { labelWidth: 190, dayWidth: 40, rowHeight: 48, paddingTop: 60, paddingBottom: 20, barHeight: 24 };

const parseDate = (str) => new Date(str + 'T00:00:00');
const getDaysBetween = (s, e) => Math.ceil((e - s) / (1000 * 60 * 60 * 24)) + 1;
const formatDate = (d) => d.toLocaleDateString('en-US', { month: 'short', day: 'numeric', weekday: 'short' });

function generateGantt(tasks, cfg) {
    const dates = [];
    tasks.forEach(t => { dates.push(parseDate(t.start)); if (t.end) dates.push(parseDate(t.end)); });
    const minDate = new Date(Math.min(...dates)), maxDate = new Date(Math.max(...dates));
    maxDate.setDate(maxDate.getDate() + 1);
    
    const totalDays = getDaysBetween(minDate, maxDate);
    const svgWidth = cfg.labelWidth + (totalDays * cfg.dayWidth);
    const svgHeight = cfg.paddingTop + (tasks.length * cfg.rowHeight) + cfg.paddingBottom;

    let html = `
        <defs>
            <style>
                .grid-line { stroke: #e2e8f0; stroke-width: 1; }
                .grid-line.bold { stroke: #cbd5e1; stroke-width: 1.5; }
                .text-hdr { font-family: sans-serif; font-size: 10px; fill: #64748b; font-weight: 600; }
                .text-lbl { font-family: sans-serif; font-size: 13px; fill: #334155; font-weight: 500; }
                .dep-line { fill: none; stroke: #94a3b8; stroke-width: 2; stroke-linejoin: round; }
                .milestone-dia { transform-box: fill-box; transform-origin: center; transform: rotate(45deg); }
            </style>
            <marker id="arrow" viewBox="0 0 10 10" refX="7" refY="5" markerWidth="5" markerHeight="5" orient="auto-start-reverse">
                <path d="M 0 1.5 L 7 5 L 0 8.5 z" fill="#94a3b8" />
            </marker>
        </defs>
        <rect width="${svgWidth}" height="${svgHeight}" fill="#ffffff" />
    `;

    // Render Timeline Header Blocks
    for (let i = 0; i < totalDays; i++) {
        const cur = new Date(minDate); cur.setDate(minDate.getDate() + i);
        const x = cfg.labelWidth + (i * cfg.dayWidth);
        if (cur.getDay() === 0 || cur.getDay() === 6) {
            html += `<rect x="${x}" y="${cfg.paddingTop}" width="${cfg.dayWidth}" height="${tasks.length * cfg.rowHeight}" fill="#f8fafc" />`;
        }
        html += `<line x1="${x}" y1="${cfg.paddingTop - 12}" x2="${x}" y2="${svgHeight - cfg.paddingBottom}" class="grid-line" />`;
        html += `<text x="${x + (cfg.dayWidth / 2)}" y="${cfg.paddingTop - 24}" text-anchor="middle" class="text-hdr">${formatDate(cur).split(',')[0]}</text>`;
        html += `<text x="${x + (cfg.dayWidth / 2)}" y="${cfg.paddingTop - 12}" text-anchor="middle" class="text-hdr" style="fill:#94a3b8">${cur.getDate()}</text>`;
    }

    // Grid Coordinates Processing Mapping Engine
    const map = {};
    tasks.forEach((t, i) => {
        const rY = cfg.paddingTop + (i * cfg.rowHeight);
        const sX = cfg.labelWidth + (getDaysBetween(minDate, parseDate(t.start)) - 1) * cfg.dayWidth;
        map[t.id] = t.type === "milestone" ? { type: "milestone", x: sX, y: rY + (cfg.rowHeight / 2) } : 
            { type: "task", x: sX, w: getDaysBetween(parseDate(t.start), parseDate(t.end)) * cfg.dayWidth, y: rY + ((cfg.rowHeight - cfg.barHeight) / 2), h: cfg.barHeight };
    });

    // Orthogonal Vector Route Lines Generator Engine
    tasks.forEach(t => {
        if (!t.dependencies) return;
        const tgt = map[t.id];
        t.dependencies.forEach(pId => {
            const prd = map[pId]; if (!prd) return;
            const sX = prd.type === "milestone" ? prd.x : prd.x + prd.w;
            const sY = prd.type === "milestone" ? prd.y : prd.y + (prd.h / 2);
            const eX = tgt.type === "milestone" ? tgt.x - 8 : tgt.x;
            const eY = tgt.type === "milestone" ? tgt.y : tgt.y + (tgt.h / 2);
            const mid = sX + 12;
            html += `<path d="M ${sX} ${sY} L ${mid} ${sY} L ${mid} ${eY} L ${eX} ${eY}" class="dep-line" marker-end="url(#arrow)" />`;
        });
    });

    // Render Row Records
    tasks.forEach((t, i) => {
        const node = map[t.id]; const rY = cfg.paddingTop + (i * cfg.rowHeight);
        let attrs = `data-name="${t.name}" data-start="${t.start}" data-type="${t.type}"`;
        if (t.type === "task") attrs += ` data-end="${t.end}" data-prog="${Math.round(t.progress * 100)}"`;

        html += `<g class="interactive-row" ${attrs} onmousemove="showTooltip(event)" onmouseleave="hideTooltip()">`;
        html += `<rect width="${svgWidth}" height="${cfg.rowHeight}" y="${rY}" fill="transparent" class="row-bg"/>`;
        html += `<line x1="0" y1="${rY}" x2="${svgWidth}" y2="${rY}" class="grid-line" />`;
        html += `<text x="15" y="${rY + (cfg.rowHeight / 2) + 5}" class="text-lbl">${t.name}</text>`;

        if (t.type === "milestone") {
            html += `<rect x="${node.x - 7}" y="${node.y - 7}" width="14" height="14" fill="${t.color}" class="milestone-dia" />`;
        } else {
            html += `<rect x="${node.x}" y="${node.y}" width="${node.w}" height="${node.h}" fill="${t.color}33" stroke="${t.color}" rx="6" />`;
            if (t.progress > 0) html += `<rect x="${node.x}" y="${node.y}" width="${node.w * t.progress}" height="${node.h}" fill="${t.color}" rx="6" />`;
        }
        html += `</g>`;
    });

    return `<svg id="gantt-svg" xmlns="http://w3.org" viewBox="0 0 ${svgWidth} ${svgHeight}">${html}</svg>`;
}

// Tooltip Controllers
const tt = document.getElementById('tooltip');
const container = document.getElementById('chart-container');

function showTooltip(e) {
    const row = e.currentTarget;
    const type = row.getAttribute('data-type');
    let content = `<strong>${row.getAttribute('data-name')}</strong><br/>`;
    content += type === 'milestone' ? `Milestone: ${row.getAttribute('data-start')}` : `Start: ${row.getAttribute('data-start')}<br/>End: ${row.getAttribute('data-end')}<br/>Progress: ${row.getAttribute('data-prog')}%`;
    
    tt.innerHTML = content; tt.style.opacity = '1';
    const box = container.getBoundingClientRect();
    tt.style.left = `${e.clientX - box.left + 15}px`; tt.style.top = `${e.clientY - box.top + 15}px`;
}
function hideTooltip() { tt.style.opacity = '0'; }

// Download Trigger Export Handler Engine
function downloadSVG() {
    const sStr = new XMLSerializer().serializeToString(document.getElementById('gantt-svg'));
    const blob = new Blob([sStr], { type: 'image/svg+xml;charset=utf-8' });
    const a = document.createElement('a'); a.href = URL.createObjectURL(blob); a.download = 'gantt_schedule.svg';
    document.body.appendChild(a); a.click(); document.body.removeChild(a);
}

function computeAndPrintCriticalPath(tasksData) {
    // 1. Prepare working data dictionary
    const nodes = {};
    tasksData.forEach(t => {
        // Calculate raw duration in days
        const start = new Date(t.start + 'T00:00:00');
        const end = t.end ? new Date(t.end + 'T00:00:00') : start;
        const duration = Math.ceil((end - start) / (1000 * 60 * 60 * 24)) + 1;

        nodes[t.id] = {
            id: t.id,
            name: t.name,
            duration: t.type === 'milestone' ? 0 : duration,
            predecessors: t.dependencies || [],
            successors: [],
            ES: 0, EF: 0, LS: 0, LF: 0
        };
    });

    // Map structural inverse successor relationships
    Object.values(nodes).forEach(node => {
        node.predecessors.forEach(predId => {
            if (nodes[predId]) nodes[predId].successors.push(node.id);
        });
    });

    // Helper: Topological Sort to handle arbitrary task entry order safely
    const visited = new Set();
    const orderedIds = [];
    function visit(id) {
        if (visited.has(id)) return;
        visited.add(id);
        nodes[id].predecessors.forEach(visit);
        orderedIds.push(id);
    }
    Object.keys(nodes).forEach(visit);

    // 2. FORWARD PASS: Determine Earliest Start (ES) & Earliest Finish (EF)
    orderedIds.forEach(id => {
        const node = nodes[id];
        if (node.predecessors.length > 0) {
            node.ES = Math.max(...node.predecessors.map(pId => nodes[pId].EF));
        } else {
            node.ES = 0;
        }
        node.EF = node.ES + node.duration;
    });

    // 3. BACKWARD PASS: Determine Latest Finish (LF) & Latest Start (LS)
    const totalProjectLength = Math.max(...Object.values(nodes).map(n => n.EF));
    
    // Process in reverse topological order
    [...orderedIds].reverse().forEach(id => {
        const node = nodes[id];
        if (node.successors.length > 0) {
            node.LF = Math.min(...node.successors.map(sId => nodes[sId].LS));
        } else {
            node.LF = totalProjectLength;
        }
        node.LS = node.LF - node.duration;
    });

    // 4. EXTRACT CRITICAL PATH: Slack/Float evaluates to 0
    const criticalNodes = Object.values(nodes).filter(node => (node.LS - node.ES) === 0);
    
    // Sort chronological order by Earliest Start timestamps
    criticalNodes.sort((a, b) => a.ES - b.ES);

    // 5. PRINT OUTPUT DIAGNOSTICS TO CONSOLE
    console.log("==================================================");
    console.log(`CRITICAL PATH CALCULATION ENGINE (Total Duration: ${totalProjectLength} days)`);
    console.log("==================================================");
    criticalNodes.forEach((node, i) => {
        const arrow = i < criticalNodes.length - 1 ? " ➔ " : "";
        console.log(`[${node.id}] "${node.name}" (Duration: ${node.duration}d)${arrow}`);
    });
    console.log("==================================================");

    return criticalNodes.map(n => n.id);
}

// Automatically trigger calculation sequence on payload runtime execution
computeAndPrintCriticalPath(tasks);

container.innerHTML = generateGantt(tasks, config);
