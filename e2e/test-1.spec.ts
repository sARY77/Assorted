import { test, expect, Page } from '@playwright/test';

const routeMapUrl = 'https://www.alaskaair.com/route-map/';
const overlayLocator = '#route-map-root > div.App > div[class^="Overlay__"]';
const viewListLocator = 'div[class^="topLeftControlsWrapper__"] > div[class^="formWrapper__"] > div[class^="toggleListWrapper__"] > a[class^="desktopListToggleButton__"]';
const routeItemsLocator = 'div[class^="UnorderedListView__"] > div[class^="ListViewWrapper__"] > div[class^="listWrapper__"] > ul[class^="ListOfRoutes__"] > li[class^="ListItem__"]';
const itemLinkLocator = 'div[class^="booking__"] > a';
const originLabel = 'Set as origin';
const destinationLabel = 'View dates';
const connectionsLocator = 'div[class^="connections__"]';
const nonstopLabel = 'Nonstop';
const resetLocator = '#content > div[class^="input_field_wrapper__"] > div[class^="react-autosuggest__"] > button[class^="input_field_clear__"]';
const routes = new Map<string, string[]>();

async function getRoutesAsync(page: Page): Promise<Map<string, string[]>> {
  await page.goto(routeMapUrl);
  const overlay = page.locator(overlayLocator);
  await overlay.locator(viewListLocator).click();
  const routeItems = overlay.locator(routeItemsLocator);
  const firstItemLink = routeItems.first().locator(itemLinkLocator);
  await expect(firstItemLink).toHaveText(originLabel);
  const numberOfOrigins = await routeItems.count();
  for (let originIndex = 0; originIndex < numberOfOrigins; ++originIndex) {
    // if (originIndex > 1) break;
    const originItem = routeItems.nth(originIndex);
    const itemLink = originItem.locator(itemLinkLocator);
    await expect(itemLink).toHaveText(originLabel);
    const origin = await originItem.getAttribute('id');
    if (origin === null) continue;
    console.log(`${originIndex + 1} of ${numberOfOrigins} ${origin}`);
    await itemLink.click();
    await expect(firstItemLink).toHaveText(destinationLabel);
    const destinations: string[] = [];
    const numberOfDestinations = await routeItems.count();
    for (let destinationIndex = 0; destinationIndex < numberOfDestinations; ++destinationIndex) {
      const destinationItem = routeItems.nth(destinationIndex);
      const destination = await destinationItem.getAttribute('id');
      if (destination === null) continue;
      if (await destinationItem.locator(connectionsLocator).textContent() !== nonstopLabel) break;
      console.log(`${destination}`);
      destinations.push(destination);
    }
    routes.set(origin, destinations);
    await page.waitForTimeout(1000);
    await page.locator(resetLocator).click();
  }
  return routes;
}

const airportUrl = 'https://www.airnav.com/airports/get?s={0}';
const airportInfoLocator = 'body > table:nth-child(4) > tbody > tr > td:nth-child(2) > font';
const airportInfoRegExp = new RegExp('^<b>(.+)</b><br>(.+), (.+), USA$');
const internationalAirports = ['LTO', 'SJD', 'MZT', 'PVR', 'GDL', 'ZLO', 'ZIH', 'CUN', 'BZE', 'LIR', 'SJO', 'YYJ', 'YVR', 'YLW', 'YYC', 'YEG'];
const airports = new Map<string, string>();

async function getAirportInfoAsync(page: Page, airport: string): Promise<string> {
  if (airports[airport] !== undefined) return airports[airport];
  if (internationalAirports.some(code => code === airport)) return ',,';
  await page.waitForTimeout(1000);
  await page.goto(airportUrl.replace('{0}', airport));
  const nameAndAddress = await page.locator(airportInfoLocator).innerHTML();
  const results = airportInfoRegExp.exec(nameAndAddress);
  const airportInfo = `${results?.at(1)},${results?.at(2)},${results?.at(3)}`;
  airports[airport] = airportInfo;
  console.log(`${airportInfo}`);
  return airportInfo;
}

const distanceUrl = 'http://www.gcmap.com/mapui?P={0}-{1}';
const distanceLocator ='#mdist > tfoot > tr > td.d';
const distances = new Map<string, number>();

async function getDistanceAsync(page: Page, origin: string, destination: string): Promise<number> {
  const route = origin < destination ? `${origin}${destination}` : `${destination}${origin}`;
  if (distances[route] !== undefined) return distances[route];
  await page.waitForTimeout(1000);
  await page.goto(distanceUrl.replace('{0}', origin).replace('{1}', destination));
  const miles = await page.locator(distanceLocator).textContent();
  const distance = Number(miles?.replace(' mi', '').replace(',', ''));
  distances[route] = distance;
  console.log(`${distance}`);
  return distance;
}

const results: string[] = [];

test('test', async ({ page }) => {
  for (const route of await getRoutesAsync(page)) {
    const origin = route[0];
    const originInfo = await getAirportInfoAsync(page, origin);
    for (const destination of route[1]) {
      const destinationInfo = await getAirportInfoAsync(page, destination);
      const distance = await getDistanceAsync(page, origin, destination);
      results.push(`${origin},${originInfo},${destination},${destinationInfo},${distance}`);
    }
  }
  console.clear();
  for (const result of results) console.log(result);
});
