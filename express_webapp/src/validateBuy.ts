
interface Item {
    char: string;
    cost: number;
}

const items = new Map<string, number>();
items.set('B', 1000);
items.set('F', 300);
items.set('L', 700);
items.set('R', 1200);
items.set('C', 2000);

export const getTotalCost = (content: string) => {
    let cost = 0;
    if (content[content.length - 1] !== '\n') {
        console.log('no EOL');
        return -1;
    }
    for (let i = 0; i < content.length - 1; i++) {
        if (!items.has(content[i])) {
            console.log('unknown item: ' + content[i]);
            return -1;
        }
        cost += items.get(content[i]);
    }
    return cost;
};

