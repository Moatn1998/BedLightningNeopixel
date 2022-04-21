function updateSliderNeopixelRot(element) {
    var sliderRot = document.getElementById("SliderRot").value;
    document.getElementById("textSliderValueRot").innerHTML = sliderRot;
    console.log(sliderRot);
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/sliderRot?value=" + sliderRot, true);
    xhr.send();
}
function updateSliderNeopixelGruen(element) {
    var sliderGruen = document.getElementById("SliderGruen").value;
    document.getElementById("textSliderValueGruen").innerHTML = sliderGruen;
    console.log(sliderGruen);
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/sliderGruen?value=" + sliderGruen, true);
    xhr.send();
}
function updateSliderNeopixelBlau(element) {
    var sliderBlau = document.getElementById("SliderBlau").value;
    document.getElementById("textSliderValueBlau").innerHTML = sliderBlau;
    console.log(sliderBlau);
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/sliderBlau?value=" + sliderBlau, true);
    xhr.send();
}
function toggleCheckbox(element) {
    var xhr = new XMLHttpRequest();
    if (element.checked) { xhr.open("GET", "/update?output=" + element.id + "&state=1", true); }
    else { xhr.open("GET", "/update?output=" + element.id + "&state=0", true); }
    xhr.send();
}
function toggleButton(x) {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/" + x, true);
    xhr.send();
}
function updateSelectionList(element) {
    var select = document.getElementById('animation');
    var option = select.options[select.selectedIndex];
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/selection_list?value=" + option.value, true)
    xhr.send();
}