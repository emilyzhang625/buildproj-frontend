import "./App.css";
import { useState } from "react";

function App() {
  const [grams, setGrams] = useState(0); 
  const [isCustom, setIsCustom] = useState(false); 
  
  const handleSubmit = () => {
    console.log(grams);
    console.log("making matcha");
  };

  const handleCustomInput = (e) => {
    let value = parseFloat(e.target.value);
    if (value > 4) {
      value = 4;
    } else if (value < 0) {
      value = 0;
    }
    setGrams(value); 
  };

  const handleShowCustomInput = () => {
    setIsCustom(!isCustom);
  };

  const handlePresetGrams = (value) => {
    setGrams(value);
    setIsCustom(false); 
  };

  return (
    <div>
      <h1>Matcha Maker</h1>
      <div className="matcha-buttons-container">
        <button onClick={() => handlePresetGrams(1)}>1g</button>
        <button onClick={() => handlePresetGrams(2)}>2g</button>
        <button onClick={() => handlePresetGrams(3)}>3g</button>
        <button onClick={() => handlePresetGrams(4)}>4g</button>
        <button className="show-custom" onClick={handleShowCustomInput}>
          Custom
        </button>
      </div>
      {isCustom && (
        <div className="custom-input-container">
          <input
            type="number"
            value={grams}
            min="0"
            max="4"
            step="0.1"
            onChange={handleCustomInput}
            placeholder="Enter custom value"
          />
        </div>
      )}
      <button className="go-button" onClick={handleSubmit}>
        Go!
      </button>
      <div>
        <p>Amount of matcha: {grams}g</p>
      </div>
    </div>
  );
}

export default App;
