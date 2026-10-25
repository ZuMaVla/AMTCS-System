export class ExperimentState {
    constructor() {
        this.serverStatus = "Disconnected";
        this.experimentStatus = "Unknown";
        this.experimentLength = 0;
        this.experimentProgress = -1;
        this.logs = [];
    }
}
