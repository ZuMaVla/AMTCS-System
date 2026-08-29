export class RemoteControlPresenter {
  constructor(model, view) {
    this.model = model;
    this.view = view;
    this.baseUrl = "http://192.168.50.1:8000";
    this.autoUpdate = false; // Flag to control automatic updates
    // Start the global timer once
    setInterval(() => {
        if (this.autoUpdate) {
            this.updateExperimentInfo();
        }
    }, 5000);
    this.view.disableButton("update-info-btn"); // Disable the update button on initialization
    this.view.disableButton("pause-btn"); // Disable the pause button on initialization
    this.view.disableButton("cancel-btn"); // Disable the cancel button on initialization
  }

  async checkServerHealth() {
    const url = `${this.baseUrl}/status`;

    try {
      const response = await fetch(url, {
        method: "GET",
        headers: {
          Accept: "application/json",
        },
      });

      if (!response.ok) {
        console.warn(`[CLIENT] Server returned error: ${response.status}`);
        return false;
      }

      const data = await response.json();

      if (data.status === "OK") {
        console.log("[CLIENT] Health status: OK", data);
        return true;
      } else {
        console.warn("[CLIENT] Health status not OK:", data);
        return false;
      }
    } catch (err) {
      console.error("[CLIENT] Server unreachable:", err);
      return false;
    }
  }

  async connectToServer() {
    console.log("[CLIENT] Attempting to connect to server...");
    this.model.serverStatus = "Connecting...";
    this.view.updateServerStatus(this.model.serverStatus);

    const ok = await this.checkServerHealth();
    console.log("[CLIENT] Server health check result:", ok);
    this.model.serverStatus = ok ? "Connected" : "Error";
    this.view.updateServerStatus(this.model.serverStatus);
  }

  async updateExperimentInfo() {
    try {
      const response = await fetch(`${this.baseUrl}/`, {
        method: "GET",
        headers: {
          Accept: "application/json",
          "client-api-key": this.view.getAccessCode(),
        },
      });
      const data = await response.json();

      // Update model
      this.model.experimentStatus = data.experiment_status;
      this.model.experimentLength = data.experiment_length;
      this.model.experimentProgress = data.experiment_progress;

      // Convert logs from {timestamp, text} → "[timestamp] text"
      this.model.logs = data.logs.map(
        (log) => `[${log.timestamp}] ${log.text}`,
      );

      if (data.experiment_status === 1 || data.experiment_status === 2) {
        // If the experiment is running or paused, start auto-updating logs
        if (!this.autoUpdate) {
          this.autoUpdate = true;
        }
      }
      else if (data.experiment_status === -1) {
        // If the experiment is not started, stop auto-updating logs
        this.autoUpdate = false;
      }

      // Presenter prepares the full list
      const fullList = [...this.model.logs];

      // Update view
      this.view.renderLogs(fullList);
      this.view.updateExperimentStatus(this.model.experimentStatus);
    } catch (err) {
      console.error("Failed to update experiment info:", err);
      this.view.enableAccessCodeInput(true); // Re-enable access code input on error
      this.autoUpdate = false; // Stop auto-updating on error
      this.view.enableButton("update-info-btn"); // Re-enable the update button on error
      this.view.disableButton("pause-btn"); // Disable the pause button on error
      this.view.disableButton("cancel-btn"); // Disable the cancel button on error
    }
  }

  async pauseExperiment() {
    let currentStatus = -1; // Default to "Not started"
    try {
      const response = await fetch(`${this.baseUrl}/`, {
        method: "GET",
        headers: {
          Accept: "application/json",
          "client-api-key": this.view.getAccessCode(),
        },
      });
      const data = await response.json();

      // Retrieve the current experiment status from the server response
      currentStatus = data.experiment_status;

      this.view.updateExperimentStatus(currentStatus);
    } catch (err) {
      console.error("Failed to change experiment status:", err);
    }

    switch (currentStatus) {
      case 1: // RUNNING → send PAUSE request
        try {
          const response = await fetch(`${this.baseUrl}/experiment/status_request/pause`, {
            method: "POST",
            headers: {
              "Content-Type": "application/json",
              "client-api-key": this.view.getAccessCode(),
            },
          });
          const data = await response.json();
          console.log("[CLIENT] Experiment paused:", data);
        } catch (err) {
          console.error("Failed to pause experiment:", err);
        }
        this.view.disableButton("pause-btn");
        break;
      case 2: // PAUSED → send RESUME request
        try {
          const response = await fetch(`${this.baseUrl}/experiment/status_request/resume`, {
            method: "POST",
            headers: {
              "Content-Type": "application/json",
              "client-api-key": this.view.getAccessCode(),
            },
          });
          const data = await response.json();
          console.log("[CLIENT] Experiment resumed:", data);
        } catch (err) {
          console.error("Failed to resume experiment:", err);
        }
        this.view.disableButton("pause-btn");
        break;
      default:
        console.warn("Experiment is not in a state that can be paused or resumed.");
    }

  }
}
