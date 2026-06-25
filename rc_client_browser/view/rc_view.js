export class RemoteControlView {
    renderLogs(logList) {
        const box = document.getElementById("log-box");
        box.innerHTML = "";

        for (const line of logList) {
            const p = document.createElement("p");
            p.textContent = line;
            box.appendChild(p);
        }

        box.scrollTop = box.scrollHeight;
    }

    updateServerStatus(status) {
        document.getElementById("server-status").textContent = status;
    }
    getAccessCode() {
        return document.getElementById("access-code").value.trim();
    }

    updateExperimentStatus(status) {
        document.getElementById("experiment-status").textContent = status;
    }

    renderLogs(logList) {
        const box = document.getElementById("log-box");
        box.innerHTML = "";

        for (const line of logList) {
            const p = document.createElement("p");
            p.textContent = line;
            box.appendChild(p);
        }

        box.scrollTop = box.scrollHeight;
    }
}
