const std = @import("std");

const UniqueChars = struct {
    chars: [10]u8,
    len: usize,

    pub fn slice(self: *const UniqueChars) []const u8 {
        return self.chars[0..self.len];
    }
};

fn get_unique_chars(words: []const []const u8) !UniqueChars {
    var result: UniqueChars = .{ .chars = undefined, .len = 0};
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

pub fn main(init: std.process.Init) !void {
    const io = init.io;

    var stdout = std.Io.File.stdout().writer(io, &.{});

    //    SEND
    // +  MORE
    //   -----
    //   MONEY
    const a: []const u8 = "SEND";
    const b: []const u8 = "MORE";
    const sum: []const u8 = "MONEY";
    const unique: UniqueChars = try get_unique_chars(&.{a, b, sum});
    try stdout.interface.print("{s}\n", .{unique.chars[0..unique.len]});
}
