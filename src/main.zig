const std = @import("std");
const solver = @import("cryptarithmetic_puzzle");
const solver_bt = @import("backtack_solver.zig");

pub fn main(init: std.process.Init) !void {
    const io = init.io;
    var stdout = std.Io.File.stdout().writer(io, &.{});
    // IDEA: implement an args parser
    // IDEA: handle x ammount of args e.g. a+b+c=d
    // IDEA: find all solutions via --all, otherwise default to the first one

    const words = [_][]const u8{ "SEND", "MORE", "MONEY" };
    var solutions: [1024*4][10]u8 = undefined;

    if (solver_bt.solve(&words, &solutions)) |stats| {
        try stdout.interface.print("Found {d} solution(s) in {d} nodes:\n\n", .{ stats.solution_count, stats.nodes_visites });
        const unique_slice = stats.unique_chars[0..stats.unique_len];
        for (solutions[0..stats.solution_count], 1..) |sol, sol_n| {
            try stdout.interface.print("Sol #{d}: ", .{sol_n});

            for (words, 0..) |word, w_i| {
                const num = solver.word_to_number(word, unique_slice, &sol);
                if (w_i == words.len - 1) {
                    try stdout.interface.print("= {d}\n", .{num});
                } else if (w_i == words.len - 2) {
                    try stdout.interface.print("{d} ", .{num});
                } else {
                    try stdout.interface.print("{d} + ", .{num});
                }
            }
        }
    } else {
        try stdout.interface.print("-1", .{});
    }
    try stdout.flush();
}
