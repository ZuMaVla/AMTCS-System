import { ExperimentState } from "./model/experiment.js";
import { RemoteControlView } from "./view/rc_view.js";
import { RemoteControlPresenter } from "./presenter/rc_presenter.js";

// Create MVP components
const model = new ExperimentState();
const view = new RemoteControlView();
const presenter = new RemoteControlPresenter(model, view);

// Wire UI events to Presenter
document.getElementById("update-info-btn").addEventListener("click", () => {
  presenter.updateExperimentInfo();
});

document.getElementById("connect-btn").addEventListener("click", () => {
  presenter.connectToServer();
});

document.getElementById("pause-btn").addEventListener("click", () => {
  presenter.pauseExperiment();
});

document.getElementById("cancel-btn").addEventListener("click", () => {
  presenter.cancelExperiment();
});
