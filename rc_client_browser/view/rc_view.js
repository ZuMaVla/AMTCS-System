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
        let text_exp_status = "Unknown";
        switch (status) {
            case -1: text_exp_status = "Not started"; break;
            case 0:  text_exp_status = "Unknown"; break;
            case 1:  text_exp_status = "Running"; break;
            case 2:  text_exp_status = "Paused"; break;
            case 3:  text_exp_status = "Finished"; break;
        }
        document.getElementById("experiment-status").textContent = text_exp_status;
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
