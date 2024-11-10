import "./App.css";
import { useState } from "react";

function App() {
  const [grams, setGrams] = useState(0);
  const min = 0;
  const max = 3;

  const handleSubmit = () => {
    console.log(grams);
    console.log("making matcha");
  };

  return (
    <div>
      <input
        type="number"
        value={grams}
        min={min}
        max={max}
        step="0.2"
        onChange={(e) => {
          setGrams(e.target.value);
        }}
        onKeyDown={(event) => {
          if (event.key !== "ArrowUp" && event.key !== "ArrowDown") {
            event.preventDefault();
          }
        }}
      ></input>
      <button onClick={handleSubmit}>Go!</button>
      <div>
        Please enter the amount of matcha powder in grams within the respective
        range of 0 to 3 grams.
      </div>
    </div>
  );
}

export default App;
