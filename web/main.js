document.addEventListener("DOMContentLoaded", function () {
    var engine = null;
    var locationEl = document.getElementById("location");
    var infoEl = document.getElementById("info");
    var linksEl = document.getElementById("links");
    var logEl = document.getElementById("log");

    function addLog(msg) {
        var p = document.createElement("p");
        p.textContent = msg;
        logEl.insertBefore(p, logEl.firstChild);
        if (logEl.children.length > 20) {
            logEl.removeChild(logEl.lastChild);
        }
    }

    function render() {
        var nodeId = engine.current_node();
        var name = engine.node_name(nodeId);
        var role = engine.node_role(nodeId);

        var roleNames = ["", " [Hub]", " [Gate]", " [Key]", " [Flavor]"];
        var roleStr = roleNames[role] || "";

        locationEl.innerHTML = "<h2>" + name + roleStr + "</h2>";

        /* keys */
        var keyCount = engine.key_count();
        var keys = [];
        for (var k = 0; k < keyCount; k++) {
            keys.push(engine.key_name(k));
        }
        infoEl.textContent = keyCount > 0
            ? "Keys: " + keys.join(", ")
            : "";

        /* links */
        var count = engine.refresh_links();
        linksEl.innerHTML = "";

        if (count === 0) {
            linksEl.innerHTML = "<p>No paths forward. You are stranded.</p>";
            return;
        }

        for (var i = 0; i < count; i++) {
            (function (idx) {
                var btn = document.createElement("button");
                var display = engine.link_display(idx);
                var passable = engine.link_passable(idx);

                btn.textContent = display + (passable ? "" : " [LOCKED]");
                btn.className = "link-btn" + (passable ? "" : " locked");
                btn.disabled = !passable;

                btn.addEventListener("click", function () {
                    var ok = engine.move(idx);
                    if (ok) {
                        addLog("Traveled via: " + display);
                        render();
                    } else {
                        addLog("The way is blocked.");
                    }
                });

                linksEl.appendChild(btn);
            })(i);
        }
    }

    DreamGraph().then(function (Module) {
        engine = {
            init: Module.cwrap("engine_init", null, ["number"]),
            current_node: Module.cwrap("engine_current_node", "number", []),
            node_name: Module.cwrap("engine_node_name", "string", ["number"]),
            node_role: Module.cwrap("engine_node_role", "number", ["number"]),
            refresh_links: Module.cwrap("engine_refresh_links", "number", []),
            link_display: Module.cwrap("engine_link_display", "string", ["number"]),
            link_passable: Module.cwrap("engine_link_passable", "number", ["number"]),
            move: Module.cwrap("engine_move", "number", ["number"]),
            key_count: Module.cwrap("engine_key_count", "number", []),
            key_name: Module.cwrap("engine_key_name", "string", ["number"]),
            node_count: Module.cwrap("engine_node_count", "number", []),
            visited: Module.cwrap("engine_visited", "number", ["number"])
        };

        var seed = Math.floor(Math.random() * 0xFFFFFFFF);
        engine.init(seed);
        addLog("World generated. Seed: " + seed);
        render();
    });
});
