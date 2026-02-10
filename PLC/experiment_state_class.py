import json
from dataclasses import dataclass, field, asdict
from typing import List

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

class ExperimentState:
    def __init__(self):
        self.experimentParameters = ExperimentParameters()
        self.experimentProgressIndex = -1

    def serialise(self) -> str:
        """Converts the current state to a JSON string."""
        # asdict() converts the nested dataclass into a dictionary automatically
        state_dict = {
            "experimentParameters": asdict(self.experimentParameters),
            "experimentProgressIndex": self.experimentProgressIndex
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
                
        except (json.JSONDecodeError, TypeError) as e:
            print(f"Error parsing JSON: {e}")

