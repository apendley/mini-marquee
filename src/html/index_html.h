const char index_html[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>MiniMarquee</title>
    <link rel="stylesheet" href="styles.css">
    <script>
        document.addEventListener('DOMContentLoaded', () => {
            document.getElementById('textColor').addEventListener('change', function () {
                const selectedOption = this.options[this.selectedIndex];
                const color = selectedOption.getAttribute('data-color');

                if (color) {
                    document.body.style.backgroundColor = color;
                }
            });
        });
    </script>
</head>
<body style="background-color: {{BGCOLOR}};">
    <div class="form-container">
        <form action="/update" method="post">
            <label for="message">Set message:</label>
            <input type="text" id="message" name="message">

            <label for="textColor">Text Color:</label>
            <select id="textColor" name="textColor">
                <option value="0"{{CS0}} data-color="{{C0}}">{{CN0}}</option>
                <option value="1"{{CS1}} data-color="{{C1}}">{{CN1}}</option>
                <option value="2"{{CS2}} data-color="{{C2}}">{{CN2}}</option>
                <option value="3"{{CS3}} data-color="{{C3}}">{{CN3}}</option>
                <option value="4"{{CS4}} data-color="{{C4}}">{{CN4}}</option>
                <option value="5"{{CS5}} data-color="{{C5}}">{{CN5}}</option>
                <option value="6"{{CS6}} data-color="{{C6}}">{{CN6}}</option>
                <option value="7"{{CS7}} data-color="{{C7}}">{{CN7}}</option>
                <option value="8"{{CS8}} data-color="{{C8}}">{{CN8}}</option>
                <option value="9"{{CS9}} data-color="{{C9}}">{{CN9}}</option>
            </select>

            <label for="brightness">Brightness:</label>
            <select id="brightness" name="brightness">
                <option value="0"{{BS0}}>{{BN0}}</option>
                <option value="1"{{BS1}}>{{BN1}}</option>
                <option value="2"{{BS2}}>{{BN2}}</option>
                <option value="3"{{BS3}}>{{BN3}}</option>
                <option value="4"{{BS4}}>{{BN4}}</option>
            </select>            

            <label for="speed">Speed:</label>
            <select id="speed" name="speed">
                <option value="0"{{TS0}}>{{TN0}}</option>
                <option value="1"{{TS1}}>{{TN1}}</option>
                <option value="2"{{TS2}}>{{TN2}}</option>
                <option value="3"{{TS3}}>{{TN3}}</option>
                <option value="4"{{TS4}}>{{TN4}}</option>
            </select>

            <label for="font">Font:</label>
            <select id="font" name="font">
                <option value="0"{{FS0}}>{{FN0}}</option> 
                <option value="1"{{FS1}}>{{FN1}}</option> 
                <option value="2"{{FS2}}>{{FN2}}</option>
                <option value="3"{{FS3}}>{{FN3}}</option>
            </select>

            <label for="rotation">Rotation:</label>
            <select id="rotation" name="rotation">
                <option value="0"{{RS0}}>{{RN0}}</option>
                <option value="1"{{RS1}}>{{RN1}}</option>            
                <option value="2"{{RS2}}>{{RN2}}</option>
                <option value="3"{{RS3}}>{{RN3}}</option>
            </select>
            <br>            
            
            <button type="submit">Update!</button>
        </form>
    </div>
</body>
</html>
)rawliteral";
