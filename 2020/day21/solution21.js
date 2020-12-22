import {splitIntoLines} from '../util.js';
export default main21;

const FOOD_REGEX = /^(?<ingredients>[a-z ]+) \(contains (?<allergens>[a-z ,]+)\)$/;

class Food {
    constructor(ingredients, allergens) {
        this.ingredients = ingredients;
        this.allergens = allergens;
    }
}

function main21(input) {
    const lines = splitIntoLines(input);
    const foods = [];
    const ingredientsToAllergens = new Map();
    for (const line of lines) {
        const match = line.match(FOOD_REGEX);
        if (match === null) {
            continue;
        }
        const ingredients = match.groups.ingredients.split(' ');
        const allergens = match.groups.allergens.split(', ');
        foods.push(new Food(ingredients, allergens));
        for (const ingredient of ingredients) {
            if (!ingredientsToAllergens.has(ingredient)) {
                ingredientsToAllergens.set(ingredient, new Set());
            }
            // Naively mark every ingredient of this food as possibly
            // having every allergen.
            allergens.forEach(allergen => ingredientsToAllergens.get(
                ingredient).add(allergen));
        }
    }

    for (const food of foods) {
        for (const [ingredient, possibleAllergens] of ingredientsToAllergens) {
            if (!food.ingredients.includes(ingredient)) {
                // Since this ingredient isn't in this food, it can't
                // contain any of this food's allergens.
                food.allergens.forEach(
                    allergen => possibleAllergens.delete(allergen));
            }
        }
    }

    let safeIngredientsCount = 0;
    for (const food of foods) {
        for (const ingredient of food.ingredients) {
            if (!ingredientsToAllergens.has(ingredient)
                || ingredientsToAllergens.get(ingredient).size === 0) {
                ++safeIngredientsCount;
                ingredientsToAllergens.delete(ingredient);
            }
        }
    }

    // Create an inverse mapping of allergens
    // to possible ingredients that contain them.
    const allergensToIngredients = new Map();
    for (const [ingredient, possibleAllergens] of ingredientsToAllergens) {
        for (const allergen of possibleAllergens) {
            if (!allergensToIngredients.has(allergen)) {
                allergensToIngredients.set(allergen, new Set());
            }
            allergensToIngredients.get(allergen).add(ingredient);
        }
    }

    while (ingredientsToAllergens.size > 0) {
        // Look for ingredients that can only contain one allergen.
        for (const [ingredient, possibleAllergens] of ingredientsToAllergens) {
            if (possibleAllergens.size <= 1) {
                ingredientsToAllergens.delete(ingredient);
            }
            if (possibleAllergens.size === 1) {
                const allergen = Array.from(possibleAllergens)[0];
                allergensToIngredients.set(allergen, new Set([ingredient]));
                console.log(ingredient, 'contains', allergen);
                // Remove this allergen from any other ingredients
                ingredientsToAllergens.forEach(
                    allergens => allergens.delete(allergen));
            }
        }
        // Look for allergens that can only be in one ingredient.
        for (const [allergen, possibleIngredients] of allergensToIngredients) {
            if (possibleIngredients.size === 1) {
                const ingredient = Array.from(possibleIngredients)[0];
                ingredientsToAllergens.delete(ingredient);
                // Remove this allergen from any other ingredients.
                ingredientsToAllergens.forEach(
                    allergens => allergens.delete(allergen));
            }
        }
    }

    const sortedAllergens = Array.from(allergensToIngredients.keys()).sort();
    // Join ingredients in order into a single string
    const dangerousIngredients = sortedAllergens.map(
        allergen => Array.from(allergensToIngredients.get(
            allergen))[0]).join(',');

    return {'Part 1': safeIngredientsCount, 'Part 2': dangerousIngredients};
}
