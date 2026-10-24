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

fn word_to_number(word: []const u8, unique_chars: []const u8, assigned_digits: []u8) u64{
    // TODO: theres a number assigned to the char, set it... where? where does
    // the structure connected to the char live, in main?
    //
    // all the words to a number, once they have been assigned?
    var result: u64 = 0;
    for (word) |char| {
        const digit: u8 = char_to_digit(char, unique_chars, assigned_digits);
        result = result * 10 + digit;
    }
    return result;
}

fn assign_digits(a: []const u8, b: []const u8, c: []const u8, index: u8,
    unique_chars: []const u8, assigned_digits: []u8, used_chars: []bool) bool {
    // TODO: implement
    // go through every char in unique chars, look for a number between 0-9
    // and then set it in a, b and c
    // backtrack if it returns false and try a new number

    // Base case
    if (index == unique_chars.len) { // you assigned numbers to all the chars
        const num_a = word_to_number(a, unique_chars, assigned_digits);
        const num_b = word_to_number(b, unique_chars, assigned_digits);
        const num_c = word_to_number(c, unique_chars, assigned_digits);
        const words = [_][]const u8{a, b, c};
        for (words) |word| {
            if (char_to_digit(word[0], unique_chars, assigned_digits) == 0) {
                return false;
            }
        }
        return (num_a + num_b == num_c);
    }
    for (0..10) |d| {
        if (!used_chars[d]) {
            assigned_digits[index] = @intCast(d);
            used_chars[d] = true;
            if (assign_digits(a, b, c, index + 1, unique_chars, assigned_digits, used_chars)) {
                return true;
            }
            // backtrack here
            used_chars[d] = false;
        }
    }
    return false;
}

pub fn main(init: std.process.Init) !void {
    const io = init.io;

    var stdout = std.Io.File.stdout().writer(io, &.{});
    // TODO: receive string in argv "a+b=c", split it into three
    // IDEA: implement an args parser
    //    SEND
    // +  MORE
    //   -----
    //   MONEY

    const a: []const u8 = "SEND";
    const b: []const u8 = "MORE";
    const c: []const u8 = "MONEY";
    var assigned_digits: [10]u8 = undefined;
    var used_digits: [10]bool = @splat(false);
    const unique: UniqueChars = try get_unique_chars(&.{ a, b, c });
    // try stdout.interface.print("{s}\n", .{unique.slice()});
    if (assign_digits(a, b, c, 0, unique.slice(), &assigned_digits, &used_digits)) {
        const num_a = word_to_number(a, unique.slice(), &assigned_digits);
        const num_b = word_to_number(b, unique.slice(), &assigned_digits);
        const num_c = word_to_number(c, unique.slice(), &assigned_digits);
        try stdout.interface.print("{d} + {d} = {d}\n", .{num_a, num_b, num_c});
    } else {
        try stdout.interface.print("no sol", .{});
    }
    try stdout.flush();
}
