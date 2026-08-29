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
            p.textContent = line;

            // Spectrum measured → green
            if (line.includes("Spectrum measured")) {
                p.classList.add("has-text-success", "has-text-weight-semibold");
            }

            // New target set → bold
            else if (line.includes("New target has been set")) {
                p.classList.add("has-text-weight-bold");
            }

            // Optional: paused → red italic
            else if (line.includes("Experiment is paused")) {
                p.classList.add("has-text-danger", "is-italic");
            }

            // Optional: resumed → blue italic
            else if (line.includes("Experiment is resumed")) {
                p.classList.add("has-text-info", "is-italic");
            }

            box.appendChild(p);
        }

        box.scrollTop = box.scrollHeight;
    }
}
