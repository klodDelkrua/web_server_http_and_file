async function testAPI(url) {
    const resultElement = document.getElementById('api-result');
    resultElement.textContent = 'Chargement...';

    try {
        const response = await fetch(url);
        const data = await response.json();
        resultElement.textContent = JSON.stringify(data, null, 2);
    } catch (error) {
        resultElement.textContent = 'Erreur: ' + error.message;
    }
}

//Test automatique au chargement
window.addEventListener('DOMContentLoaded', () => {
   console.log('Page chargee, Serveur C++ actif !');
});