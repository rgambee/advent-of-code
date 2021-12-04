use std::fmt;

pub struct PartialSolution {
    pub message: String,
    pub answer: i64,
}

pub struct Solution(pub Option<PartialSolution>, pub Option<PartialSolution>);

impl fmt::Display for PartialSolution {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}: {}", self.message, self.answer)
    }
}

impl fmt::Display for Solution {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        if self.0.is_some() {
            writeln!(f, "PART 1\n{}", self.0.as_ref().unwrap())?;
        }
        if self.1.is_some() {
            writeln!(f, "PART 2\n{}", self.1.as_ref().unwrap())?;
        }
        Ok(())
    }
}
