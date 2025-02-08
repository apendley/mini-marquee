const char styles_css[] = R"rawliteral(
body, input, button, select, textarea, label {
    font-family: Verdana, Geneva, sans-serif;
}

body {
    display: flex;
    justify-content: center;
    align-items: center;
    margin: 0;
}

.form-container {
    padding: 16px;
}

form {
    background: #fff;
    padding: 8px 40px 8px 40px;
    border-radius: 8px;
    box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
    display: inline-block;
    width: auto;
}

label {
    display: block;
    margin-bottom: 8px;
    font-weight: bold;
    text-align: left;
    width: 100%;
    font-size: 1.0rem;
    padding-top: 8px;
}

.checkbox-container {
    display: flex;
    align-items: center;
    justify-content: start;    
}

.change-color {
    display: inline-block; 
}

.checkbox {
    display: inline-block;
    vertical-align: bottom;
}

.small-text {
    font-size: 0.7rem;
    color: #666;
    text-align: left;
    padding-top: 0px;
}

select {
    width: 90%;
    padding: 0px 8px 0px 8px;
    margin: 0 auto 4px auto;
    display: block;
    border: 1px solid #ccc;
    border-radius: 4px;
    color: #333;
    font-size: 1.0rem;
}

input[type="text"],
select {
    width: 90%;
    padding: 8px;
    margin: 0 auto 4px auto;
    display: block;
    border: 1px solid #ccc;
    border-radius: 4px;
}

input[type="text"] {
    font-size: 1.0em;
}

button {
    padding: 10px 20px;
    background-color: #28a745;
    color: white;
    border: none;
    border-radius: 20px;
    cursor: pointer;
    transition: background-color 0.3s, box-shadow 0.3s;
    display: block;
    margin: 0 auto;
    font-size: 1.0em;
}

button:hover {
    background-color: #218838;
    box-shadow: 0 2px 5px rgba(0, 0, 0, 0.2);
}

button:focus {
    outline: none;
    box-shadow: 0 0 0 3px rgba(40, 167, 69, 0.5);
}
)rawliteral";
