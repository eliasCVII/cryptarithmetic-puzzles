const std = @import("std");

const UniqueChars = struct {
    chars: [10]u8,
    len: usize,

    pub fn slice(self: *const UniqueChars) []const u8 {
        return self.chars[0..self.len];
    }
};

fn get_unique_chars(words: []const []const u8) !UniqueChars {
    var result: UniqueChars = .{ .chars = undefined, .len = 0 };
    var seen: u32 = 0;

    for (words) |word| {
        for (word) |char| {
            const bit = @as(u32, 1) << @intCast(char - 'A');
            if ((seen & bit) == 0) {
                seen |= bit;
                if (result.len >= 10) return error.TooManyUniqueChars;
                result.chars[result.len] = char;
                result.len += 1;
            }
        }
    }
    return result;
}

fn char_to_digit(char: u8, unique_chars: []const u8, assigned_digits: []u8) u8 {
    // getter, returns the value in assigned_digits related to unique_chars
    for (unique_chars, 0..) |c, i| {
        if (c == char) return assigned_digits[i];
    }
    unreachable;
}

fn find_index(char: u8, unique_chars: []const u8) usize {
    for (unique_chars, 0..) |c, i| {
        if (c == char) return i;
    }
    unreachable;
}

fn word_to_number(word: []const u8, unique_chars: []const u8, assigned_digits: []u8) u64 {
    // all the words to a number, once they have been assigned?
    var result: u64 = 0;
    for (word) |char| {
        const digit: u8 = char_to_digit(char, unique_chars, assigned_digits);
        result = result * 10 + digit;
    }
    return result;
}

fn compute_weights(words: []const []const u8, unique_chars: []const u8, factor: []const i64) [10]i64 {
    var weights: [10]i64 = @splat(0);
    for (words, factor) |word, f| {
        var m: i64 = f;
        for (0..word.len) |i| {
            const ch: u8 = word[word.len - i - 1];
            weights[find_index(ch, unique_chars)] += m;
            m *= 10;
        }
    }
    return weights;
}

fn assign_digits(weights: []const i64, index: u8, current_sum: i64, unique_chars: []const u8, assigned_digits: []u8, used_mask: u16) bool {
    // go through every char in unique chars, look for a number between 0-9
    // and then set it in a, b and c
    // backtrack if it returns false and try a new number

    // Base case
    if (index == unique_chars.len) { // you assigned numbers to all the chars
        return current_sum == 0;
    }
    for (0..10) |d| {
        if ((used_mask & @as(u16, 1) << @intCast(d)) != 0) continue;
        assigned_digits[index] = @intCast(d);
        const next_sum = current_sum + weights[index] * @as(i64, @intCast(d));
        if (assign_digits(weights, index + 1, next_sum, unique_chars, assigned_digits, used_mask | (@as(u16, 1) << @intCast(d)))) {
            return true;
        }
    }
    return false;
}

pub fn main(init: std.process.Init) !void {
    const io = init.io;

    var stdout = std.Io.File.stdout().writer(io, &.{});
    // IDEA: implement an args parser
    // IDEA: handle x ammount of args e.g. a+b+c=d
    // IDEA: find all solutions via --all, otherwise default to the first one

    const a: []const u8 = "SEND";
    const b: []const u8 = "MORE";
    const c: []const u8 = "MONEY";
    var assigned_digits: [10]u8 = undefined;
    const unique: UniqueChars = try get_unique_chars(&.{ a, b, c });
    const weights: [10]i64 = compute_weights(&.{ a, b, c }, unique.slice(), &.{ 1, 1, -1 });

    if (assign_digits(&weights, 0, // index
        0, // current_sum
        unique.slice(), // array of unique chars found
        &assigned_digits, // numbers tried
        0 // used_mask
    )) {
        const num_a = word_to_number(a, unique.slice(), &assigned_digits);
        const num_b = word_to_number(b, unique.slice(), &assigned_digits);
        const num_c = word_to_number(c, unique.slice(), &assigned_digits);
        try stdout.interface.print("{d} + {d} = {d}\n", .{ num_a, num_b, num_c });
    } else {
        try stdout.interface.print("-1", .{});
    }
    try stdout.flush();
}
