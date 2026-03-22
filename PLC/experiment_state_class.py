import json
from dataclasses import dataclass, field, asdict
from typing import List
from enum import Enum


@dataclass
class ExperimentParameters:
    sampleCode: str = ""
    Ts: List[str] = field(default_factory=list)         # individual empty list for each instance
    StartWL: int = 0
    DG: int = 0
    DGRangeNo: int = 0
    NA: int = 0
    slits: int = 0
    maxAT: int = 0
    isCRRemoval: bool = False

class StepName(Enum):
    TEMPERATURE = "Go to temperature"
    SPECTRUM = "Acquire spectrum"

class StepStatus(Enum):
    WAITING = 0
    REQUESTED = 1
    COMPLETED = 2

class InitStep(Enum):
    ID = 0
    CURRENT_T = 1
    SETPT_T = 2
    CONTROL = 3

@dataclass
class ExperimentStep: 
    action: StepName 
    status: StepStatus

 


@dataclass
class ExperimentCycle:
    T: ExperimentStep
    S: ExperimentStep

class ExperimentFlow:
    def __init__(self):
        self.cycles: List[ExperimentCycle] = []                                  # empty list 
        
    def populate(self, num_cycles: int):
        self.cycles.clear()                                                      # Clear existing cycles before populating
        for i in range(num_cycles):
            self.cycles.append(
                ExperimentCycle(
                    T=ExperimentStep(StepName.TEMPERATURE, StepStatus.WAITING),
                    S=ExperimentStep(StepName.SPECTRUM, StepStatus.WAITING)
                )
            )

    def update_step_status_by_experimentProgressIndex(self, cycle_index: int):   # To be used after recovery from iHR320 
        for i in range(cycle_index + 1):        
            self.cycles[i].T.status = StepStatus.COMPLETED
            self.cycles[i].S.status = StepStatus.COMPLETED
        self.cycles[cycle_index + 1].T.status = StepStatus.WAITING
        self.cycles[cycle_index + 1].S.status = StepStatus.WAITING

    def PLC_mode(self, cycle_index: int) -> ExperimentStep | None:
        if len(self.cycles) <= cycle_index + 1:                                  # End of experiment, all steps completed
            return None
        step = self.cycles[cycle_index + 1].T                                    # First step of a cicle -> Go to temperature
        if step.status == StepStatus.COMPLETED:
            step = self.cycles[cycle_index + 1].S                                # Second step of a cicle -> Acquire spectrum
        return step                          


class ExperimentState:
    def __init__(self):
        self.experimentParameters = ExperimentParameters()
        self.experimentFlow = ExperimentFlow()
        self.experimentProgressIndex = -1
        self.experimentLength = len(self.experimentParameters.Ts)

    def serialise(self) -> str:
        """Converts the current state to a JSON string."""
        # asdict() converts the nested dataclass into a dictionary automatically
        state_dict = {
            "experimentParameters": asdict(self.experimentParameters),
            "experimentProgressIndex": self.experimentProgressIndex,
            "experimentLength": self.experimentLength,
        }
        return json.dumps(state_dict)

    def deserialise(self, json_string: str):
        """Updates the state from a JSON string."""
        try:
            data = json.loads(json_string)
            
            # Retrieve the experiment progress index
            self.experimentProgressIndex = data.get("experimentProgressIndex", -1)

            # Retrieve the experiment parameters
            param_data = data.get("experimentParameters", {})
            if param_data:
                for key, value in param_data.items():
                    if hasattr(self.experimentParameters, key):
                        setattr(self.experimentParameters, key, value)

            # Retrieve the experiment length
            self.experimentLength = data.get("experimentLength", 0)
            
                
        except (json.JSONDecodeError, TypeError) as e:
            print(f"Error parsing JSON: {e}")

