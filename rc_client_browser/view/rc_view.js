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
        this.updateButtons(status);
    }

    disableButton(buttonId) {
        const button = document.getElementById(buttonId);
        if (button) {
            button.disabled = true;
        }
    }

    updateButtons(status) {
        const btn = document.getElementById("pause-btn");
        const btnCancel = document.getElementById("cancel-btn");

        if (status === -1 || status === 0 || status === 3) {
            // NOT STARTED or UNKNOWN or FINISHED → disable button
            btn.textContent = "⏸";
            btn.disabled = true;
            btnCancel.disabled = true;
        } 
        else if (status === 1) { 
            // RUNNING → show PAUSE button
            btn.textContent = "⏸";
            btn.classList.remove("is-success");
            btn.classList.add("is-warning");
            btn.disabled = false;
            btnCancel.disabled = false;
        } 
        else if (status === 2) { 
            // PAUSED → show RESUME button
            btn.textContent = "▶";
            btn.classList.remove("is-warning");
            btn.classList.add("is-success");
            btn.disabled = false;
            btnCancel.disabled = true;
        }
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
