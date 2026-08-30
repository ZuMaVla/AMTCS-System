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
        if (status === "Connected") {
            this.enableButton("update-info-btn");
        } else {
            this.disableButton("update-info-btn");
        }
    }

    getAccessCode() {
        return document.getElementById("access-code").value.trim();
    }

    enableAccessCodeInput(enable) {
        const accessCodeInput = document.getElementById("access-code");
        accessCodeInput.disabled = !enable;
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

    enableButton(buttonId) {
        const button = document.getElementById(buttonId);
        if (button) {
            button.disabled = false;
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
            this.enableAccessCodeInput(true);
        } 
        else if (status === 1) { 
            // RUNNING → show PAUSE button
            btn.textContent = "⏸";
            btn.classList.remove("is-success");
            btn.classList.add("is-warning");
            btn.disabled = false;
            btnCancel.disabled = false;
            this.enableAccessCodeInput(false);
        } 
        else if (status === 2) { 
            // PAUSED → show RESUME button
            btn.textContent = "▶";
            btn.classList.remove("is-warning");
            btn.classList.add("is-success");
            btn.disabled = false;
            btnCancel.disabled = true;
            this.enableAccessCodeInput(false);
        }
    }


    renderLogs(logList) {
        const box = document.getElementById("log-box");
        box.innerHTML = "";

        for (const line of logList) {
            const p = document.createElement("p");

            // Split by fixed length
            const timestamp = line.slice(0, 22);
            const logMessage = line.slice(22);

            // Timestamp span (always standard)
            const tsSpan = document.createElement("span");
            tsSpan.textContent = timestamp;
            tsSpan.classList.add("has-text-grey-dark");

            // Message span (Bulma styling)
            const msgSpan = document.createElement("span");
            msgSpan.textContent = logMessage;

            if (logMessage.includes("Spectrum measured")) {
                msgSpan.classList.add("has-text-success", "has-text-weight-semibold");
            }
            else if (logMessage.includes("New target has been set")) {
                msgSpan.classList.add("has-text-weight-bold", "has-text-grey-dark");
            }
            else if (logMessage.includes("Experiment is paused")) {
                msgSpan.classList.add("has-text-danger", "is-italic");
            }
            else if (logMessage.includes("Experiment is resumed")) {
                msgSpan.classList.add("has-text-info", "is-italic");
            }
            else {
                msgSpan.classList.add("has-text-grey-dark");
            }

            p.appendChild(tsSpan);
            p.appendChild(msgSpan);
            box.appendChild(p);
        }

        box.scrollTop = box.scrollHeight;
    }

    confirmCancelExperiment() {
        return window.confirm("Are you sure you want to cancel the experiment?");
    }

}
